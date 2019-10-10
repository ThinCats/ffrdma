package utils

import (
	"fmt"
	"os"
	"strconv"
	"time"
)

// CreateDir creates a dir in dirname
// if Exist, create dir with specific prefix
func CreateDir(dirname string, prefix string) (newName string) {
	if prefix == "" {
		prefix = strconv.Itoa(int(time.Now().Unix()))[:10]
	}

	_, err := os.Stat(dirname)
	if err == nil {
		dirname = fmt.Sprintf("%s_%s", dirname, prefix)
	}

	if _, err := os.Stat(dirname); os.IsNotExist(err) {
		os.Mkdir(dirname, os.ModePerm)
	} else {
		if err != nil {
			panic(err)
		}
		panic("Dir already Exist")
	}
	return dirname
}


