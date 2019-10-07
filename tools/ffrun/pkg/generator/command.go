package generator

import (
	"ffrun/pkg/algorithm"
	"ffrun/pkg/types"
	"strconv"
	"strings"
)

// CmdConfig is use for generate shell command
type CmdConfig struct {
	Program   string
	Args      string
	HostMapNp types.HostMapNumproc
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

// CmdResultGenerator is a generator to generate sequence of string
func CmdResultGenerator(cfg CmdConfig) func() (result *CmdResult, end bool) {
	// construct port map
	var hostMapPorts = make(types.HostMapPorts)
	for host, np := range cfg.HostMapNp {
		hostMapPorts[host] = algorithm.PortSelect(host, np, 10000)
	}
	piSlice := algorithm.CalculateRank(hostMapPorts)

	i := 0
	return func() (*CmdResult, bool) {
		// start to generate
		if i < len(piSlice) {
			i++
			return &CmdResult{
				Program:   cfg.Program,
				Args:      cfg.Args,
				Pi:        piSlice[i-1],
				HostPorts: hostMapPorts,
			}, false
		}
		return nil, true
	}
}
