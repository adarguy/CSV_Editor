# File: csveditor.py

#Author: Adar Guy
#contact: adarguy10@gmail.com

#!/opt/bin/python3


import sys
import csv
import re

fileName = ''
numRows = 0
numCols = 0
contents = []

def checkPmtrs(par1, type1, par2=0, type2=''):
	if type1 in 'row':	
		if par1 >= numRows or par1 < 0:
			raise Exception('Specified row ('+str(par1)+') is not in list range')
	if type1 in 'col':
		if par1 >= numCols or par1 < 0:
			raise Exception('Specified col ('+str(par1)+') is not in list range')
	if type2 in 'row':	
		if par2 >= numRows or par2 < 0:
			raise Exception('Specified row ('+str(par2)+') is not in list range')
	if type2 in 'col':
		if par2 >= numCols or par2 < 0:
			raise Exception('Specified col ('+str(par2)+') is not in list range')

def help():        
	print( "\nThe valid commands are:\n"
	"quit                     -- to exit the program\n"
	"help                     -- to display this help message\n"
	"load <filename>          -- to read in a spreadsheet\n"
	"save                     -- to save the spreadsheet back\n"
	"save <filename>          -- to save back to a different file\n"
	"merge <filename>         -- to read and append rows from another file\n"
	"stats                    -- to report on the spreadsheet size\n"
	"sort <col>               -- sort rows based on text data in column <col>\n"
	"sortnumeric <col>        -- sort rows based on numbers in column <col>\n"
	"deleterow <n>            -- delete row <n> from spreadsheet\n"
	"findrow <col> <text>     -- print the number of the first row\n"
	"                            such that column <col> holds <text>\n"
	"findrow <col> <text> <n> -- similarly, except search starts at\n"
	"                            row number n\n"
	"printrow <n>             -- print row number <n>\n"
	"printrow <n> <m>         -- print rows numbered <n> through <m>\n"
	"evalsum <col>            -- print the sum of the numbers that\n"
	"                            are in column <col>\n"
	"evalavg <col>            -- print the average of the numbers that\n"
	"                            are in column <col>\n")
def load(fN):
	global fileName
	fileName = fN
	csvfile = open(fileName, newline='')
	csvreader = csv.reader(csvfile, delimiter=',')
	global contents
	contents = []
	global numRows
	numRows = 0
	for row in csvreader:
		contents.append(row)
		if len(contents[numRows]) != len(contents[0]):
			raise Exception('Specified row ('+ str(numRows) +') has different number of columns from first row')
		numRows+= 1
	global numCols
	numCols = len(row)  
	csvfile.close()

def save(fN=''):
	if fN == '':
		fN = fileName
	csvfile = open(fN, 'w', newline='')
	csvwriter = csv.writer(csvfile, delimiter=',')
	for row in contents:
		csvwriter.writerow(row)
	csvfile.close()

def merge(fN):
	csvfile = open(fN, newline='')
	csvreader = csv.reader(csvfile, delimiter=',')
	tempRows=0
	global contents
	for row in csvreader:
		if numCols != len(row):
			raise Exception('Files ('+fileName+') and ('+fN+') have unequal row lengths')
		contents.append(row)
		tempRows+= 1     
	csvfile.close()
	global numRows
	numRows+=tempRows

def stats():
	print('File: ' + fileName + '\nRows: ' + str(numRows) + '\nColumns: ' + str(numCols))

def sort(colNum):
	checkPmtrs(colNum, 'col')
	global contents
	contents = sorted(contents, key=lambda x: re.sub('(\s|_|-)','',(x[colNum]).lower()))
def sortnumeric(colNum):
	checkPmtrs(colNum, 'col')
	global contents
	try:
		contents = sorted(contents, key=lambda x: int(re.sub('(\s|_|-)','', x[colNum])))
	except:
		raise Exception('Specified column ('+str(colNum)+') contains non-integer values')

def deleterow(rowNum):
	checkPmtrs(rowNum, 'row')
	global numRows
	global contents
	contents.pop(rowNum)	
	numRows-=1

def findrow(colNum, element, fromRow=0):
	checkPmtrs(colNum, 'col', fromRow, 'row')
	colList = [row[colNum] for row in contents]	
	if fromRow != 0:
		for x in range(fromRow):
			colList[x] = '\\'
	try:		
		return '-- found in row ' + str(colList.index(element))
	except:
		raise Exception('No matching row was found')

def evalsum(colNum):
	checkPmtrs(colNum, 'col')
	try:
		colSum=0
		colList = [float(row[colNum]) for row in contents]
		for x in colList:
			colSum += x
		return colSum
	except:
		raise Exception('Specified column ('+colNum+') contains non-Integer values')

def evalavg(colNum):
	return evalsum(colNum)/numRows

def printrow(rowStart, rowEnd):
	checkPmtrs(rowStart, 'row', rowEnd, 'row')
	for x in range(rowStart, rowEnd+1):
		print(re.sub('[_-]', ' ', '|'.join(map(str, contents[x]))))

def main():
	while True:
		try:
			subcommand = input('Enter a subcommand ==> ')
			subcommand = (subcommand.replace('"','')).lower()
			try:
				subcommand, parameter1, parameter2, parameter3 = subcommand.split()
			except:
				try:
					subcommand, parameter1, parameter2 = subcommand.split()
				except:
					try:
						subcommand, parameter1 = subcommand.split()
					except:
						pass
			if subcommand in ('quit'):
				print('Program Exited')
				break
			elif subcommand in ('help'):
				help()
			elif subcommand in ('load'):
				try:
					load(parameter1)
				except IndexError:
					raise Exception('File is empty or does not exist')
			elif subcommand in ('save'):
				try:
					save(parameter1)
				except:
					save()
			elif subcommand in ('merge'):
				merge(parameter1)
			elif subcommand in ('stats'):
				stats()				
			elif subcommand in ('sort'):
				sort(int(parameter1))
			elif subcommand in ('sortnumeric'):
				sortnumeric(int(parameter1))
			elif subcommand in ('deleterow'):
				deleterow(int(parameter1))			
			elif subcommand in ('findrow'):
				if parameter3 != None:
					print(findrow(int(parameter1), parameter2, int(parameter3)))
				else:
					print(findrow(int(parameter1), parameter2))
			elif subcommand in ('printrow'):
				if parameter2==None:
					parameter2 = parameter1
				printrow(int(parameter1), int(parameter2))
			elif subcommand in ('evalsum'):
				print(evalsum(int(parameter1)))
			elif subcommand in ('evalavg'):
				print(evalavg(int(parameter1)))			
			else:
				print('-- unrecognized command (' + subcommand +'), type help for a list of commands\n')			
		except:
			print('Error occured while accessing the file '+fileName)
			reason = sys.exc_info()
			if 'NoneType' in str(reason[1]):
				print('Reason: Missing required parameters')
			else:
				print('Reason: ', reason[1])
		parameter1 = None
		parameter2 = None
		parameter3 = None

if __name__ == '__main__':
	main()










