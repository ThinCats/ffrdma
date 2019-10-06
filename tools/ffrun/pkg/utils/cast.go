package utils

import "strconv"

// SliceIntToString convert int slice to string slice
func SliceIntToString(slice []int) (res []string) {
	for _, val := range slice {
		res = append(res, strconv.Itoa(val))
	}
	return
}
