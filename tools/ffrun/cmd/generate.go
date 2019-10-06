package cmd

import (
	"ffrun/pkg/algorithm"
	"ffrun/pkg/types"
	"ffrun/pkg/utils"
	"fmt"
	"os"
	"path"

	"github.com/mkideal/cli"
)

// writerGenerator will generate diffrent
// writer according to arguments
func writerGenerator(outdir string) func(int) *utils.EWriter {
	// no outdir
	if outdir == "" {
		return func(int) *utils.EWriter {
			return utils.NewEWriter(os.Stdout)
		}
	}
	outdir = utils.CreateDir(outdir, "")
	return func(rank int) *utils.EWriter {
		w, err := os.Create(path.Join(outdir, fmt.Sprintf("Rank%d.sh", rank)))
		if err != nil {
			panic(err)
		}
		return utils.NewEWriter(w)
	}
}

// cmdString generate one command string
func cmdString(program string, pi types.ProcessInfo, hostMap types.HostMapPorts) string {
	var str string
	str += fmt.Sprintf("# Rank %d\n", pi.Rank)
	str += fmt.Sprintf("%s --r_myIp %s --r_myPort %d --r_hostmap %s\n", program, pi.Host, pi.Port, hostMap.String())
	return str
}

func generateCmd(hostMapNp types.HostMapNumproc, program string, outdir string) {
	writerGen := writerGenerator(outdir)
	// construct port map
	var hostMapPorts = make(types.HostMapPorts)
	for host, np := range hostMapNp {

		hostMapPorts[host] = algorithm.PortSelect(host, np, 10000)
	}
	piSlice := algorithm.CalculateRank(hostMapPorts)

	// start to generate
	for _, pi := range piSlice {
		writer := writerGen(pi.Rank)
		str := cmdString(program, pi, hostMapPorts) + "\n"
		_, err := writer.WriteString(str)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		writer.Flush()
	}
}

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

type generateT struct {
	cli.Helper
	HostConfig hostConfigDecoder `cli:"*H,Host" usage:"Specify the Host, like node1:2,node2:4"`
	Program    string            `cli:"*p,program" usage:"the program to launch"`
	OutDir     string            `cli:"o,out-dir" dft:"\"\"" usage:"the output dir for generated files (rank1.sh rank0.sh). If not set, will output to stdout"`
}

// Generate is a sub command
var Generate = &cli.Command{
	Name: "generate",
	Desc: "generate boot script\n" +
		"Example:\n" +
		"    ffrun generate -H node110:3,node112:4 -p ./a.out -o size7_node2",
	Argv: func() interface{} { return new(generateT) },
	Fn: func(ctx *cli.Context) error {
		argv := ctx.Argv().(*generateT)
		generateCmd(argv.HostConfig.hosts, argv.Program, argv.OutDir)
		return nil
	},
}
