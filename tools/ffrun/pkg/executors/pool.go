package executors

import (
	"context"
	"sync"
	"time"

	"github.com/sirupsen/logrus"
)

type PoolState int

const (
	INIT PoolState = iota
	STARTING
	END
)

// TaskConfig defines config
type TaskConfig struct {
	// Ctx     context.Context
	Program  string
	Args     []string
	Logger   *logrus.Entry
	WaitTime time.Duration
	// if pool is to end, the max time can wait for a process
	BeforeHook ProcessHookFunc
	AfterHook  ProcessHookFunc
}

// Clone a new config
func (c *TaskConfig) Clone() TaskConfig {
	return TaskConfig{
		Program:    c.Program,
		Args:       c.Args,
		Logger:     c.Logger,
		WaitTime:   c.WaitTime,
		BeforeHook: c.BeforeHook,
		AfterHook:  c.AfterHook,
	}
}

// Pool contains pool of executors
type Pool struct {
	Logger *logrus.Entry

	tasks       []TaskConfig
	wg          *sync.WaitGroup
	notifyCh    chan struct{}
	ctx         context.Context
	cancelFn    context.CancelFunc
	minWaitTime time.Duration
}

// NewPool ...
func NewPool(ctx context.Context, logger *logrus.Logger) *Pool {
	if logger == nil {
		logger = logrus.New()
	}
	pool := &Pool{}
	pool.Logger = logger.WithFields(logrus.Fields{})
	pool.wg = &sync.WaitGroup{}
	pool.notifyCh = make(chan struct{})
	pool.ctx, pool.cancelFn = context.WithCancel(ctx)
	pool.minWaitTime = 3600 * time.Hour

	return pool
}

// AddTask adds a task
func (p *Pool) AddTask(cfg TaskConfig) {
	p.tasks = append(p.tasks, cfg)
	if cfg.WaitTime < p.minWaitTime {
		p.minWaitTime = cfg.WaitTime
	}
}

// Run ...
func (p *Pool) Run() {
	for _, config := range p.tasks {
		exeCfg := ProcessExecutionConfig{
			ProgramName: config.Program,
			Args:        config.Args,
			Logger:      config.Logger.WithContext(p.ctx),
			Wg:          p.wg,
			NotfiyCh:    p.notifyCh,
			BeforeHook:  config.BeforeHook,
			AfterHook:   config.AfterHook,
		}
		p.wg.Add(1)
		go StartProcess(p.ctx, exeCfg)
	}

	// wait for one of process stop
	<-p.notifyCh
	p.Stop()
}

// Stop all the process
func (p *Pool) Stop() {
	select {
	case p.notifyCh <- struct{}{}:
	default:
	}
	// p.Logger.Debugf("Wait %d ms for all process\n", p.minWaitTime.Milliseconds())
	time.Sleep(p.minWaitTime)
	p.cancelFn()
	p.wg.Wait()
}
