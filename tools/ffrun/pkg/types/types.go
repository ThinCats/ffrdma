package types

import (
	"errors"
	"ffrun/pkg/utils"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// HostMapNumproc map host to np
type HostMapNumproc map[string]int

// HostMapPorts map host to portList
type HostMapPorts map[string][]int

// ProcessInfo ...
type ProcessInfo struct {
	Host string
	Port int
	Rank int
}

// ProcessInfoSlice ...
type ProcessInfoSlice []ProcessInfo

func (s ProcessInfoSlice) Len() int      { return len(s) }
func (s ProcessInfoSlice) Swap(i, j int) { s[i], s[j] = s[j], s[i] }
func (s ProcessInfoSlice) Less(i, j int) bool {
	if s[i].Host < s[j].Host {
		return true
	} else if s[i].Host == s[j].Host {
		return s[i].Port < s[j].Port
	}
	return false
}

// String strings h, like "10.10.0.100:100+200+300,10.10.0.101:100+200+500"
func (h HostMapPorts) String() string {
	hs := []string{}
	for host, ports := range h {
		str := fmt.Sprintf("%s:%s", host, strings.Join(utils.SliceIntToString(ports), "+"))
		hs = append(hs, str)
	}
	return strings.Join(hs, ",")
}

// FromString deserialize from string
func (h HostMapPorts) FromString(str string) (HostMapPorts, error) {
	if str == "" {
		return nil, nil
	}
	// node110:100+200+300,node112:200+400+300
	res := make(HostMapPorts)
	hostPortsList := strings.Split(str, ",")
	for _, hostPorts := range hostPortsList {
		sHostPorts := strings.Split(hostPorts, ":")
		if len(sHostPorts) != 2 {
			return nil, errors.New("Invalid format in --hostmap")
		}
		host := sHostPorts[0]
		portsListStr := strings.Split(sHostPorts[1], "+")

		var portsList []int
		for _, portStr := range portsListStr {
			port, err := strconv.Atoi(portStr)
			if err != nil {
				return nil, errors.New("Invalid format in --hostmap")
			}
			portsList = append(portsList, port)
		}
		res[host] = portsList
	}
	return res, nil
}

// FromString deserialize from string
func (h HostMapNumproc) FromString(str string) (HostMapNumproc, error) {
	if str == "" {
		return nil, nil
	}
	res := make(HostMapNumproc)
	hostPortList := strings.Split(str, ",")
	for _, hostPort := range hostPortList {
		sHostPort := strings.Split(hostPort, ":")
		if len(sHostPort) != 2 {
			return nil, errors.New("Invalid format in -H")
		}
		host := sHostPort[0]
		port, err := strconv.Atoi(sHostPort[1])
		if err != nil {
			return nil, err
		}
		if _, ok := res[host]; ok {
			fmt.Fprintf(os.Stderr, "Warning, %s is duplicated, will ignore one\n\n", host)
			continue
		}
		res[host] = port
	}
	return res, nil
}
