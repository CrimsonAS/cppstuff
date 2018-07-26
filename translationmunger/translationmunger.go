package main

import (
	"encoding/xml"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
)

type TsMessageLocation struct {
	FileName string `xml:"filename,attr"`
	Line     int    `xml:"line,attr"`
}

type TsMessageTranslation struct {
	Type string `xml:"type,attr"`
	Text string `xml:",chardata"`
}

type TsMessage struct {
	Location    TsMessageLocation    `xml:"location"`
	Source      string               `xml:"source"`
	Translation TsMessageTranslation `xml:"translation"`
}

type TsContext struct {
	Name    string      `xml:"name"`
	Message []TsMessage `xml:"message"`
}

type TsHeader struct {
}

type TsDocument struct {
	XMLName  xml.Name
	Version  string      `xml:"version,attr"`
	Language string      `xml:"language,attr"`
	Contexts []TsContext `xml:"context"`
}

var reverseMode = flag.Bool("reverse", false, "reverse strings")
var longMode = flag.Bool("long", false, "long strings")

func main() {
	outPath := flag.String("o", "", "output filename (default overwrites input)")
	flag.Parse()
	inputPaths := flag.Args()

	if len(inputPaths) < 1 || (!*reverseMode && !*longMode) {
		fmt.Fprintf(os.Stderr, "Usage: %s [-reverse|-long] [flags] input\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(1)
	}

	for _, path := range inputPaths {
		opath := *outPath
		if opath == "" {
			opath = path
		}

		if err := MungeTSFile(path, opath); err != nil {
			fmt.Fprintf(os.Stderr, "error processing '%s': %s\n", path, err)
			os.Exit(1)
		}

		fmt.Printf("munged %s to %s\n", path, opath)
	}
}

func MungeTSFile(inPath, outPath string) error {
	b, err := ioutil.ReadFile(inPath)
	if err != nil {
		return err
	}

	var ts TsDocument
	xml.Unmarshal(b, &ts)

	for idx := range ts.Contexts {
		for midx := range ts.Contexts[idx].Message {
			message := &ts.Contexts[idx].Message[midx]

			// This MungeLikeTS might be unnecessary (done below). I forget.
			message.Source = MungeLikeTS(message.Source)

			// Mark it unfinished
			message.Translation.Type = "unfinished"

			translationText := message.Source

			// Reverse it...
			if *reverseMode {
				translationText = Reverse(translationText)
			}

			// Make the translation really long...
			if *longMode {
				translationText = translationText + translationText + translationText
			}

			message.Translation.Text = translationText
		}
	}

	ts.XMLName = xml.Name{Local: "TS"}
	buf, err := xml.MarshalIndent(&ts, "", "    ")
	if err != nil {
		panic("Error marshalling XML")
	}

	header := []byte("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
	header = append(header, []byte("<!DOCTYPE TS>\n")...)
	buf = append(header, buf...)
	buf = []byte(MungeLikeTS(string(buf)))

	if err := ioutil.WriteFile(outPath, buf, 0644); err != nil {
		return err
	}

	return nil
}

func MungeLikeTS(s string) string {
	return strings.Replace(s, "&#xA;", "\n", -1)
}

func Reverse(s string) string {
	ret := make([]byte, len(s))

	for idx := 0; idx < len(s); idx++ {
		c := s[idx]
		if c == '%' {
			start := idx // the %code starts here
			idx += 1     // skip % symbol
			for ; idx < len(s); idx++ {
				nc := s[idx]
				if (nc <= '0' || nc >= '9') && nc != 'L' {
					// probably the end of the sequence
					break
				}
			}

			// the %code ends at 'idx'
			thing := s[start:idx]

			// we want to write 'thing' un-reversed into the position ret[i
			for nidx := 0; nidx < len(thing); nidx++ {
				ret[len(s)-(idx-nidx)] = thing[nidx]
			}

			if idx < len(s) {
				ret[len(s)-1-idx] = s[idx]
			}
		} else {
			ret[len(s)-1-idx] = c
		}
	}

	return string(ret)
}
