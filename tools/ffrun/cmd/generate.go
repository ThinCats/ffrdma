package cmd

import (
	"ffrun/pkg/generator"
	"ffrun/pkg/types"
	"fmt"
	"strings"

	"github.com/mkideal/cli"
)

func generateCmd(hostMapNp types.HostMapNumproc, program string, args string, outdir string) {
	cfg := generator.CmdConfig{
		Program:   program,
		Args:      args,
		HostMapNp: hostMapNp,
	}

	cmdResultGen := generator.CmdResultGenerator(cfg)
	writerGen := generator.DirFileWriterGenerator(outdir)
	var (
		res *generator.CmdResult
		end bool
	)

	for {
		res, end = cmdResultGen()
		if end {
			break
		}
		writer := writerGen(fmt.Sprintf("Rank%d.sh", res.Pi.Rank))
		str := fmt.Sprintf("# Rank%d\n", res.Pi.Rank)
		str += strings.Join(res.ToArgv(), " ") + "\n"
		_, err := writer.WriteString(str)
		if err != nil {
			panic(err)
		}
		writer.Flush()
	}

}

type generateT struct {
	cli.Helper
	HostConfig hostConfigDecoder `cli:"*H,Host" usage:"Specify the Host, like node1:2,node2:4"`
	Program    string            `cli:"*p,program" usage:"the program to launch"`
	OutDir     string            `cli:"o,out-dir" dft:"" usage:"the output dir for generated files (rank1.sh rank0.sh). If not set, will output to stdout"`
}

// Generate is a sub command
var Generate = &cli.Command{
	Name: "generate",
	Desc: "generate boot script\n\n" +
		"Example:\n" +
		"    ffrun generate -H node110:3,node112:4 -p ./a.out -o size7_node2 -- good hello world" +
		"After --, the remaining args will be treated as args for generated program\n",
	Argv: func() interface{} { return new(generateT) },
	Fn: func(ctx *cli.Context) error {
		argv := ctx.Argv().(*generateT)
		generateCmd(argv.HostConfig.hosts, argv.Program, strings.Join(ctx.Args(), " "), argv.OutDir)
		return nil
	},
}
