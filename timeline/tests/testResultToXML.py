#!/usr/bin/python
# -*- coding: UTF-8 -*-
# this script is used to convert dt test result to xml format which is used to display in jenkins

from xml.dom.minidom import parse
import time

def GetTestResult(filepath):
    dom = parse(filepath)
    testsuitesElem = dom.getElementsByTagName('testsuites')[0]
    return (int(testsuitesElem.getAttribute('tests')), int(testsuitesElem.getAttribute('failures')))

# testResult = (totalCount, failedCount)
def WriteResultToTemplate(templateFilepath, resultFilepath, testResult):
    dom = parse(templateFilepath)
    dom.getElementsByTagName('Passed')[0].firstChild.nodeValue = testResult[0] - testResult[1]
    dom.getElementsByTagName('Failed')[0].firstChild.nodeValue = testResult[1]
    dom.getElementsByTagName('tcCount')[0].firstChild.nodeValue = testResult[0]
    dom.getElementsByTagName('PassRate')[0].firstChild.nodeValue = float(testResult[0] - testResult[1]) / float(testResult[0]) 
    fp = open(resultFilepath, 'w', encoding='utf-8')
    dom.writexml(fp, indent='', addindent='', newl='', encoding='utf-8')
    fp.close()

if __name__ == '__main__':
    testResultFile = 'rawTestResult.xml'
    xmlTemplateFile = 'testResultTemplate.xml'
    xmlResultFile = 'testResult.xml'
    WriteResultToTemplate(xmlTemplateFile, xmlResultFile, GetTestResult(testResultFile))