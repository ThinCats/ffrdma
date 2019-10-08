package generator

import (
	"ffrun/pkg/algorithm"
	"ffrun/pkg/types"
	"strconv"
	"strings"

	"github.com/sirupsen/logrus"
)

// CmdConfig is use for generate shell command
type CmdConfig struct {
	Program   string
	Args      string
	HostMapNp types.HostMapNumproc
	HostPorts types.HostMapPorts
}

// CmdResult is the result of parsed cmd
type CmdResult struct {
	Program   string
	Args      string
	Pi        types.ProcessInfo
	HostPorts types.HostMapPorts
}

// ToArgv converts cmd result to argv
func (r *CmdResult) ToArgv() []string {
	// str += fmt.Sprintf("# Rank %d\n", r.Pi.Rank)
	argv := []string{r.Program, "--r_myip", r.Pi.Host, "--r_myport", strconv.Itoa(r.Pi.Port), "--r_hostmap", r.HostPorts.String(), "--"}
	argv = append(argv, strings.Split(r.Args, " ")...)
	return argv
}

// CmdResultGen generates cmd reuslt
type CmdResultGen func() (result *CmdResult, end bool)

// CmdResultGenerator is a generator to generate sequence of string
func CmdResultGenerator(cfg CmdConfig, localHost string) func() (result *CmdResult, end bool) {
	// construct port map
	var hostMapPorts types.HostMapPorts
	if cfg.HostPorts == nil {
		hostMapPorts = make(types.HostMapPorts)
		for host, np := range cfg.HostMapNp {
			hostMapPorts[host] = algorithm.PortSelect(host, np, 10000)
		}
	} else {
		hostMapPorts = cfg.HostPorts
	}
	piSlice := algorithm.CalculateRank(hostMapPorts)

	logrus.Info(piSlice, localHost)
	i := 0
	return func() (*CmdResult, bool) {
		for i < len(piSlice) {
			// allow multi matching
			if localHost != "*" && piSlice[i].Host != localHost {
				i++
			} else {
				break
			}
		}
		if i >= len(piSlice) {
			return nil, true
		}

		i++
		return &CmdResult{
			Program:   cfg.Program,
			Args:      cfg.Args,
			Pi:        piSlice[i-1],
			HostPorts: hostMapPorts,
		}, false
	}
}
