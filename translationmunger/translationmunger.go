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

	codeStart := -1

	for idx, c := range s {
		if codeStart >= 0 {
			if (c <= '0' || c >= '9') && c != 'L' {
				// end of code sequence
				copy(ret[len(ret)-idx:], s[codeStart:idx])
				codeStart = -1
			} else {
				// skip until the end of the sequence
				continue
			}
		}
		if codeStart < 0 && c == '%' {
			codeStart = idx
			continue
		}

		cb := []byte(string(c))

		// Copy the rune into its reversed position. If the rune is multiple bytes,
		// those bytes will not be reversed.
		copy(ret[len(ret)-len(cb)-idx:], cb)
	}
	if codeStart >= 0 {
		copy(ret, s[codeStart:])
	}

	return string(ret)
}
