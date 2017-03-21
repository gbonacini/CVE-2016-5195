all: dcow

dcow: ./src/main/main.go ./src/expl/expl.go
	GOPATH=`pwd` go build -ldflags='-s -w -extldflags "-static"' -o dcow main
run:
	GOPATH=`pwd` go run  ./src/main/main.go
clean:
	@rm -f dcow 
