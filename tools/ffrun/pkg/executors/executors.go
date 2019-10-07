package executors

import (
	"sync"

	"github.com/go-cmd/cmd"
	"github.com/sirupsen/logrus"
)

// ProcessHookFunc defines the hook function type
type ProcessHookFunc func(cfg *ProcessExecutionConfig, status cmd.Status)

// ProcessExecutionConfig represents how a process run
type ProcessExecutionConfig struct {
	ProgramName string
	Args        []string
	Logger      *logrus.Entry
	Wg          *sync.WaitGroup
	NotfiyCh    chan struct{}
	BeforeHook  ProcessHookFunc
	AfterHook   ProcessHookFunc
}

// Clone will clone a config without inner state
func (p *ProcessExecutionConfig) Clone() ProcessExecutionConfig {
	return ProcessExecutionConfig{
		ProgramName: p.ProgramName,
		Args:        p.Args,
		Logger:      nil,
		Wg:          p.Wg,
		NotfiyCh:    p.NotfiyCh,
	}
}
