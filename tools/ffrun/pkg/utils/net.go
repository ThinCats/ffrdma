package utils

import (
	"net"
)

func ListAllLocalAddress() []string {
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		panic(err)
	}
	res := []string{}

	for _, a := range addrs {
		if ipnet, ok := a.(*net.IPNet); ok {
			res = append(res, ipnet.IP.String())
		}
	}
	return res
}

func IsIn(addr string, addrSet []string) bool {
	for _, val := range addrSet {
		if addr == val {
			return true
		}
	}
	return false
}
