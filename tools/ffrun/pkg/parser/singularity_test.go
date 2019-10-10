package parser

import (
	"reflect"
	"testing"
)

func TestSingularity_parse(t *testing.T) {
	type args struct {
		args []string
	}
	tests := []struct {
		name    string
		s       *Singularity
		args    args
		want    *FormatType
		wantErr bool
	}{
		{
			name: "Test 1",
			s:    nil,
			args: args{
				args: []string{"../../singularity", "exec", "ubuntu.sif", "bash", "-l", "-c", "\"ls\""},
			},
			want: &FormatType{
				Head: "../../singularity exec ubuntu.sif bash",
				Tail: "-l -c \"ls\"",
			},
			wantErr: false,
		},
		{
			name: "Test 2",
			s:    nil,
			args: args{
				args: []string{"singularity", "exec", "-f", "--install-dir", "ubuntu.sif", "bash", "-l", "-c"},
			},
			want: &FormatType{
				Head: "singularity exec -f --install-dir ubuntu.sif bash",
				Tail: "-l -c",
			},
			wantErr: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			s := &Singularity{}
			got, err := s.parse(tt.args.args)
			if (err != nil) != tt.wantErr {
				t.Errorf("Singularity.parse() error = %v, wantErr %v", err, tt.wantErr)
				return
			}
			if !reflect.DeepEqual(got, tt.want) {
				t.Errorf("Singularity.parse() = %+v, want %+v", got, tt.want)
			}
		})
	}
}
