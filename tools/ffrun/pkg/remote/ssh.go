package remote

import (
	"strings"
	"sync"
	"time"

	"github.com/appleboy/easyssh-proxy"
	"github.com/sirupsen/logrus"
)

// SshRunConfig ...
type SshRunConfig struct {
	Hostname string
	Program  string
	Args     string
}

func runBySSH(cfg SshRunConfig) (stdout string, stderr string, done bool, err error) {
	ssh := &easyssh.MakeConfig{
		Server:  cfg.Hostname,
		Timeout: 5 * time.Second,
	}

	stdout, stderr, done, err = ssh.Run(strings.Join([]string{cfg.Program, cfg.Args}, " "))
	return stdout, stderr, done, err
}

func RunHostsBySSH(logger *logrus.Logger, program string, args string, hosts []string) {
	var wg sync.WaitGroup
	for _, host := range hosts {
		ssh := &easyssh.MakeConfig{
			Server:  host,
			User:    "rdma_match",
			KeyPath: "/home/rdma_match/.ssh/id_rsa",
			Port:    "22",
		}
		wg.Add(1)
		go func(host string) {
			defer wg.Done()
			_, stderr, done, err := ssh.Run(program + " " + args)
			if err != nil {
				logger.Errorf("Start ssh in hostname: %s failed: %s\n%s", host, err, stderr)
				panic("No!!")
			}
			if done {
				logger.Debugln(host, "ran ssh command")
			}
		}(host)
	}
	wg.Wait()
}
