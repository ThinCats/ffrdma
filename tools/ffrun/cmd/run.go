package cmd

import (
	"context"
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

func runCmd(ctx context.Context, hostMapNp types.HostMapNumproc, program string, args string) {

	cfg := generator.CmdConfig{
		Program:   program,
		Args:      args,
		HostMapNp: hostMapNp,
	}

	logger := ctx.Value("logger").(*logrus.Logger)

	cmdResultGen := generator.CmdResultGenerator(cfg)

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

		pool.AddTask(cfg)
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
	HostConfig hostConfigDecoder `cli:"*H,Host" usage:"Specify the Host, like node1:2,node2:4"`
	Program    string            `cli:"*p,program" usage:"the program to launch"`
	Debug      bool              `cli:"d,debug" dft:"false" usage:"enable debug log"`
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
		runCmd(cmdCtx, argv.HostConfig.hosts, argv.Program, strings.Join(ctx.Args(), " "))
		ctx.String(strings.Join(ctx.Args(), " "))
		return nil
	},
}
