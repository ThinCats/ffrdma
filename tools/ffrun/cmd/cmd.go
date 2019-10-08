package cmd

import (
	"ffrun/pkg/types"
)

type hostConfigDecoder struct {
	hosts types.HostMapNumproc
}

// 10.10.0.110:5,10.10.0.112:4
func (d *hostConfigDecoder) Decode(s string) error {
	var err error
	d.hosts, err = d.hosts.FromString(s)
	if err != nil {
		return err
	}
	return nil
}

type hostMapPortsDecoder struct {
	hosts types.HostMapPorts
}

func (d *hostMapPortsDecoder) Decode(s string) error {
	var err error
	d.hosts, err = d.hosts.FromString(s)
	return err
}
