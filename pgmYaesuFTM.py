#!/usr/bin/python
# Convert a csv file from chirp into an XML file that can be
# converted to a dat 
#

import csv
import xml.etree.cElementTree as ET
from optparse import OptionParser
from subprocess import call

# /ftm-import channels.xml ~/Desktop/MEMFTM100D.dat MEMFTM100D.dat > file.xml

def ImportData(input_file,output_file):
	call(['./ftm-import','output.xml', input_file, output_file])
	
def ConvertToXML(input_file):
	root=ET.Element("channels")
	reader=csv.DictReader(open(input_file,'r'), delimiter=',')

	for row in reader:
		channum=row['Location']
		chan=ET.SubElement(root, 'channel')
		chan.set("bank",str(1))
		chan.set("slot",str(channum))
		f=row['Frequency']
		f=f[0:7]
		ET.SubElement(chan, "band").text = "VHF"
		ET.SubElement(chan, "offset").text = row['Duplex'] + row['Offset']
		ET.SubElement(chan, "sql").text = 'T-TX' 
		ET.SubElement(chan, "tone").text = row['rToneFreq']
		ET.SubElement(chan, "power").text = "high"
		ET.SubElement(chan, "tag").text = row['Name']
		ET.SubElement(chan, "frequency").text = f

	tree= ET.ElementTree(root)
	tree.write('output.xml')

if __name__ == "__main__":
    usage = """

    This script will convert a CSV from chirp into an XML file
    suitable for use with the Yaesu FTM100DR (and probably FTM400DR)

    %prog -i /path/to/input.csv -i /path/to/old/MEMFTM100D.dat -o /path/to/new/MEMFTM100D.dat

    Place the resulting MEMFTM100D.dat file on your sd card and load it into your radio:

    Menu: SD Card: Backup: Read from backup

    """

    parser = OptionParser(usage)
    parser.add_option("-o", dest="output_dat", help="Path to dat outfile")
    parser.add_option("-d", dest='input_dat', help="Path to dat infile")
    parser.add_option("-i", dest="input_csv", help="Path to csv infile")

    (options, args) = parser.parse_args()
    if (options.output_dat is None) or (options.input_dat is None) or (options.input_csv is None):
	print(usage)
    else:
        ConvertToXML(options.input_csv)
        ImportData(options.input_dat,options.output_dat)
