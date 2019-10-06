package algorithm

import (
	"math/rand"
	"time"
)

// PortSelect will selects num ports in host
func PortSelect(host string, num int, minPort int) []int {
	rand.Seed(time.Now().Unix() + int64(len(host)))
	if 65535-minPort+1 < num {
		panic("PortSelect: minPort can't satisfy num")
	}
	// TODO: Implement this function, not just randomly choose
	res := []int{}
	cnt := 0
	for _, port := range rand.Perm(65535) {
		if cnt >= num {
			break
		}
		// only accept those port greater or equal than minPort
		if port >= minPort {
			res = append(res, port)
			cnt++
		}
	}
	return res
}
