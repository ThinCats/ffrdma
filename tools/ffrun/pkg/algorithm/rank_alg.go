package algorithm

import (
	"ffrun/pkg/types"
	"sort"
)

// CalculateRank calculates rank
func CalculateRank(hostMapPorts types.HostMapPorts) types.ProcessInfoSlice {
	piSlice := types.ProcessInfoSlice{}

	// add to processInfo
	for host, ports := range hostMapPorts {
		for _, port := range ports {
			piSlice = append(piSlice, types.ProcessInfo{Host: host, Port: port})
		}
	}

	// sort
	sort.Sort(piSlice)

	// add to rank
	for i := range piSlice {
		piSlice[i].Rank = i
	}

	return piSlice
}
