package cmd

import (
	"context"
	"errors"
	"ffrun/pkg/executors"
	"ffrun/pkg/generator"
	"ffrun/pkg/parser"
	"ffrun/pkg/types"
	"ffrun/pkg/utils"
	"strings"
	"time"

	"github.com/go-cmd/cmd"
	"github.com/mkideal/cli"
	"github.com/sirupsen/logrus"
)

type runCmdConfig struct {
	Program       string
	Args          string
	HostMapNp     types.HostMapNumproc
	HostPorts     types.HostMapPorts
	IsSingularity bool
}

func filterHost(hosts []string) (localHost string, remoteHosts []string) {
	localAllAddress := utils.ListAllLocalAddress()
	var findFlag bool = false
	for _, host := range hosts {
		// use findFlag to avoid many useless lookup
		if findFlag || !utils.IsIn(host, localAllAddress) {
			remoteHosts = append(remoteHosts, host)
		} else {
			localHost = host
			findFlag = true
		}
	}
	return localHost, remoteHosts
}

func runCmd(ctx context.Context, cmdCfg runCmdConfig) {

	cfg := generator.CmdConfig{
		Program:   cmdCfg.Program,
		Args:      cmdCfg.Args,
		HostMapNp: cmdCfg.HostMapNp,
		HostPorts: cmdCfg.HostPorts,
	}

	logger := ctx.Value("logger").(*logrus.Logger)

	var hosts []string

	if cmdCfg.HostPorts != nil {
		for host := range cmdCfg.HostPorts {
			hosts = append(hosts, host)
		}
	} else {
		for host := range cmdCfg.HostMapNp {
			hosts = append(hosts, host)
		}
	}
	// filter localhost, remotehost
	localHost, _ := filterHost(hosts)

	if len(strings.TrimSpace(localHost)) == 0 {
		logger.Errorln("can't find local host in given hosts")
		return
	}

	cmdResultGen := generator.CmdResultGenerator(cfg, localHost)

	var (
		res *generator.CmdResult
		end bool = false
	)

	pool := executors.NewPool(context.Background(), logger)
	for {
		res, end = cmdResultGen()
		if end {
			break
		}

		processLogger := logger.WithFields(logrus.Fields{
			"host": res.Pi.Host,
			"rank": res.Pi.Rank,
			"port": res.Pi.Port,
		},
		)

		argv := res.ToArgv()
		// ignore program, 1:
		cfg := executors.TaskConfig{
			Program:  argv[0],
			Args:     argv[1:],
			Logger:   processLogger,
			WaitTime: 100 * time.Millisecond,
		}

		// Only set config when rank is 0
		if res.Pi.Rank == 0 {
			cfg.BeforeHook = func(exeCfg *executors.ProcessExecutionConfig, status cmd.Status) {
				exeCfg.Logger.Infof("Program %s started.", exeCfg.ProgramName)
			}
			cfg.AfterHook = func(exeCfg *executors.ProcessExecutionConfig, status cmd.Status) {
				exeCfg.Logger.Infof("Program End: %s. Time elapsed: %f s. Exit Status: %d", exeCfg.ProgramName, status.Runtime, status.Exit)
			}
		}

		logger.Debugln("Pool Task Added")
		pool.AddTask(cfg)
	}

	if pool.TaskNum() == 0 {
		logger.Warningln("No process to run")
		return
	}

	// handle signal
	notifySignalCh := utils.NotifySignalsDefault()
	endCh := make(chan struct{})

	go func() {
		pool.Run()
		select {
		case endCh <- struct{}{}:
		default:
		}
	}()

	select {
	case <-endCh:
	case <-notifySignalCh:
		pool.Stop()
	}
	return
}

type runT struct {
	cli.Helper
	HostConfig    hostConfigDecoder   `cli:"H,Host" usage:"Specify the Host, like node1:2,node2:4"`
	Program       string              `cli:"p,program" usage:"the program to launch"`
	Debug         bool                `cli:"d,debug" dft:"false" usage:"enable debug log"`
	HostMapConfig hostMapPortsDecoder `cli:"hostmap" usage:"Specify host map to launch, node1:100+200,node2:300+400"`
	Singularity   bool                `cli:"s,singularity" usage:"Enable singularity support"`
	// Master     bool              `cli:"m,master" dft:"false" usage:"as master node, will boot up other slave node"`
}

// Run is a sub command
var Run = &cli.Command{
	Name: "run",
	Desc: "Run launch Parallel Process written by ffrdma\n\n" +
		"Example:\n" +
		"    ffrun run -H node110:3,node112:4 -p ./a.out -- -n 1 ...<program args>\n",
	Argv: func() interface{} { return new(runT) },
	Fn: func(ctx *cli.Context) error {
		argv := ctx.Argv().(*runT)
		logger := logrus.New()
		if argv.Debug {
			logger.SetLevel(logrus.DebugLevel)
		}

		cmdCtx := context.WithValue(context.Background(), "logger", logger)
		cmdCfg := runCmdConfig{
			Program:       argv.Program,
			Args:          strings.Join(ctx.Args(), " "),
			HostMapNp:     nil,
			HostPorts:     nil,
			IsSingularity: argv.Singularity,
		}
		// check mode
		if argv.HostMapConfig.hosts != nil {
			cmdCfg.HostPorts = argv.HostMapConfig.hosts
		} else if argv.HostConfig.hosts != nil {
			cmdCfg.HostMapNp = argv.HostConfig.hosts
		} else {
			return errors.New("Error: must specify -H or --hostmap")
		}

		if !argv.Singularity && argv.Program == "" {
			return errors.New("Error: must specify -p when not singularity mode")
		}
		// Reset program
		if argv.Singularity {
			sig := &parser.Singularity{}
			format, err := sig.Parse(ctx.Args())
			if err != nil {
				return err
			}
			cmdCfg.Program = format.Head
			cmdCfg.Args = format.Tail
		}

		runCmd(cmdCtx, cmdCfg)

		return nil
	},
}
