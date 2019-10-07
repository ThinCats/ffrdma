package generator

import (
	"ffrun/pkg/utils"
	"os"
	"path"
)

// DirFileWriterGenerator will generate diffrent
// writer according to arguments
func DirFileWriterGenerator(outdir string) func(string) *utils.EWriter {
	// no outdir
	if outdir == "" {
		writer := utils.NewEWriter(os.Stdout)
		return func(string) *utils.EWriter {
			return writer
		}
	}
	outdir = utils.CreateDir(outdir, "")
	return func(outfile string) *utils.EWriter {
		w, err := os.Create(path.Join(outdir, outfile))
		if err != nil {
			panic(err)
		}
		return utils.NewEWriter(w)
	}
}
