package executors

import (
	"context"
	"strings"
	"time"

	"github.com/go-cmd/cmd"
)

// Process ...
type Process struct {
	Stderr chan int
	Stdout chan int
}

// StartProcess ...
func StartProcess(ctx context.Context, cfg ProcessExecutionConfig) {
	defer cfg.Wg.Done()
	defer func() {
		// notify parent to stop
		select {
		case cfg.NotfiyCh <- struct{}{}:
		default:
			// if block, continue (other goroutine is working)
		}
	}()

	cmd := cmd.NewCmdOptions(cmd.Options{
		Buffered:  false,
		Streaming: true,
	}, cfg.ProgramName, cfg.Args...)

	cfg.Logger.Debugln(cfg.ProgramName, strings.Join(cfg.Args, " "))
	cmd.Start()

	// !! Race and ugly
	go func() {
		for {
			status := cmd.Status()
			if status.PID != 0 || status.Complete || status.Error != nil {
				cfg.Logger = cfg.Logger.WithField("PID", status.PID)
				break
			}
			time.Sleep(100 * time.Millisecond)
		}
	}()

	if cfg.BeforeHook != nil {
		cfg.BeforeHook(&cfg, cmd.Status())
	}
	doneCh := cmd.Done()
	// executor control the log print
	subCtx, cancelSubFn := context.WithCancel(context.Background())
	// monitor goroutine
	go func() {
		for {
			select {
			case <-subCtx.Done():
				return
			case line := <-cmd.Stdout:
				cfg.Logger.Infoln(line)
			case line := <-cmd.Stderr:
				cfg.Logger.Errorln(line)
			}
		}
	}()

	// if program done or parent cancel
	select {
	case <-doneCh:
	case <-ctx.Done():
	}

	// wait go routine to consume all log
	for len(cmd.Stdout) > 0 || len(cmd.Stderr) > 0 {
		time.Sleep(10 * time.Millisecond)
	}

	// cacnel log go routine
	cancelSubFn()
	cmd.Stop()
	cfg.Logger.Debugln("Command Stop")
	// TODO: Figure out why not need to close these channel
	// close(cmd.Stdout)
	// close(cmd.Stderr)
	if cfg.AfterHook != nil {
		cfg.AfterHook(&cfg, cmd.Status())
	}
}
