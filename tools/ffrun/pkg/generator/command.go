package generator

import (
	"ffrun/pkg/algorithm"
	"ffrun/pkg/types"
	"fmt"
)

// CmdConfig is use for generate shell command
type CmdConfig struct {
	Program   string
	Args      string
	HostMapNp types.HostMapNumproc
}

// CmdString creates one command string
func CmdString(program string, args string, pi types.ProcessInfo, hostMap types.HostMapPorts) string {
	var str string
	str += fmt.Sprintf("# Rank %d\n", pi.Rank)
	str += fmt.Sprintf("%s --r_myip %s --r_myport %d --r_hostmap %s -- %s\n", program, pi.Host, pi.Port, hostMap.String(), args)
	return str
}

// CmdStringGenerator is a generator to generate sequence of string
func CmdStringGenerator(cfg CmdConfig) func() (str string, i int, end bool) {
	// construct port map
	var hostMapPorts = make(types.HostMapPorts)
	for host, np := range cfg.HostMapNp {
		hostMapPorts[host] = algorithm.PortSelect(host, np, 10000)
	}
	piSlice := algorithm.CalculateRank(hostMapPorts)

	i := 0
	return func() (string, int, bool) {
		// start to generate
		if i < len(piSlice) {
			i++
			return CmdString(cfg.Program, cfg.Args, piSlice[i-1], hostMapPorts), i - 1, false
		}
		return "", i, true
	}
}
