package cmd

import (
	"context"
	"errors"
	"ffrun/pkg/executors"
	"ffrun/pkg/generator"
	"ffrun/pkg/types"
	"ffrun/pkg/utils"
	"strings"
	"time"

	"github.com/go-cmd/cmd"
	"github.com/mkideal/cli"
	"github.com/sirupsen/logrus"
)

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

func runCmd(ctx context.Context, hostMapNp types.HostMapNumproc, hostMapPorts types.HostMapPorts, program string, args string) {

	cfg := generator.CmdConfig{
		Program:   program,
		Args:      args,
		HostMapNp: hostMapNp,
		HostPorts: hostMapPorts,
	}

	logger := ctx.Value("logger").(*logrus.Logger)

	var hosts []string

	if hostMapPorts != nil {
		for host := range hostMapPorts {
			hosts = append(hosts, host)
		}
	} else {
		for host := range hostMapNp {
			hosts = append(hosts, host)
		}
	}
	// filter localhost, remotehost
	localHost, _ := filterHost(hosts)

	if len(strings.TrimSpace(localHost)) == 0 {
		logger.Errorln("can't find local host in given hosts")
		return
	}

	logger.Debugln("Generate Generator")
	cmdResultGen := generator.CmdResultGenerator(cfg, localHost)

	var (
		res *generator.CmdResult
		end bool = false
	)

	logger.Debugln("Create Pool")
	pool := executors.NewPool(context.Background(), logger)
	for {
		logger.Debugln("Getting cmdResult")
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

		// ignore program, 1:
		cfg := executors.TaskConfig{
			Program:  res.Program,
			Args:     res.ToArgv()[1:],
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
	Program       string              `cli:"*p,program" usage:"the program to launch"`
	Debug         bool                `cli:"d,debug" dft:"false" usage:"enable debug log"`
	HostMapConfig hostMapPortsDecoder `cli:"hostmap" usage:"Specify host map to launch, node1:100+200,node2:300+400"`
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
		if argv.HostMapConfig.hosts != nil {
			runCmd(cmdCtx, nil, argv.HostMapConfig.hosts, argv.Program, strings.Join(ctx.Args(), " "))
		} else if argv.HostConfig.hosts != nil {
			runCmd(cmdCtx, argv.HostConfig.hosts, nil, argv.Program, strings.Join(ctx.Args(), " "))
		} else {
			return errors.New("Error: must specify -H or --hostmap")
		}

		return nil
	},
}
