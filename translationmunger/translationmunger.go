package main

import (
	"encoding/xml"
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

func main() {
	fileName := "foobar.ts"
	xmlFile, err := os.Open(fileName)
	if err != nil {
		panic(fmt.Sprintf("Error opening file: %s", err))
	}
	defer xmlFile.Close()

	b, _ := ioutil.ReadAll(xmlFile)

	var ts TsDocument
	xml.Unmarshal(b, &ts)

	for idx := range ts.Contexts {
		for midx := range ts.Contexts[idx].Message {
			// This MungeLikeTS might be unnecessary (done below). I forget.
			ts.Contexts[idx].Message[midx].Source = MungeLikeTS(ts.Contexts[idx].Message[midx].Source)

			// Mark it unfinished
			ts.Contexts[idx].Message[midx].Translation.Type = "unfinished"

			// Reverse it...
			ts.Contexts[idx].Message[midx].Translation.Text = Reverse(ts.Contexts[idx].Message[midx].Source)

			// Make the translation really long...
			//ts.Contexts[idx].Message[midx].Translation.Text = ts.Contexts[idx].Message[midx].Translation.Text + ts.Contexts[idx].Message[midx].Translation.Text + ts.Contexts[idx].Message[midx].Translation.Text
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

	err = ioutil.WriteFile(fileName, buf, 0644)
	if err != nil {
		panic("Error writing file")
	}

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
