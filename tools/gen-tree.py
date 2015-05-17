#!/usr/bin/python

"""
Description: Tool for transforming JSON dumps from Sparrow Compiler into Graphviz dot diagrams

Copyright (c) 2015, Lucian Radu Teodorescu
"""

import os, subprocess, sys, json, argparse, cgi
# from collections import defaultdict

nodes = {}
types = {}
nodesToExpand = []  # Node to expand: (node-ref, level, expand)
nodesExpanded = {}

totalChildLinks = 0
totalRefLinks = 0
totalPropLinks = 0
totalExplLinks = 0
expandedChildLinks = 0
expandedRefLinks = 0
expandedPropLinks = 0
expandedExplLinks = 0

args = None
def parseArgs():
    global args
    parser = argparse.ArgumentParser(description='Generates dot trees from Sparrow json dumps')
    parser.add_argument('jsonFile', metavar='jsonFile', type=str,
                       help='the .json file to process')
    parser.add_argument('--open', action='store_true',
                       help='translate the dot file to pdf and open it')
    parser.add_argument('--showNodeIds', action='store_true',
                       help='shows the Ids of the nodes')
    parser.add_argument('--startNode', metavar='ID', type=int,
                       help='the node we should start expanding from')
    parser.add_argument('--maxNodes', metavar='N', type=int, default=10000,
                       help='max number of nodes to process (default: 10000)')
    parser.add_argument('--maxLevels', metavar='N', type=int, default=20,
                       help='max levels to expand (default: 20)')
    parser.add_argument('--maxChildNodes', metavar='N', type=int, default=999999,
                       help='max children nodes links to expand (default: all)')
    parser.add_argument('--maxReferredNodes', metavar='N', type=int, default=0,
                       help='max referred nodes links to expand (default: 0)')
    parser.add_argument('--maxPropNodes', metavar='N', type=int, default=0,
                       help='max properties nodes links to expand (default: 0)')
    parser.add_argument('--maxExplNodes', metavar='N', type=int, default=999999,
                       help='max explanation nodes links to expand (default: all)')
    args = parser.parse_args()


def getFileContents(filename):
    with open(filename) as f:
        return f.read().rstrip()

def getNodePropString(nodeObj, key):
    for prop in nodeObj['properties']:
        if prop['name'] == key:
            return prop['val']
    return ''

def getNodeName(nodeKind, nodeObj):
    idx = nodeKind.rfind('.')
    res = nodeKind[idx+1:]
    name = getNodePropString(nodeObj, 'name')
    if name == '':
        name = getNodePropString(nodeObj, 'spr.operation')
    return res if name == '' else res + ': ' + name

def interpretPtr(ptr):
    pType = ptr['p']
    ref = None
    obj = None
    if pType != 0:
        ref = ptr['ref']
    if pType == 1:
        obj = ptr['obj']
    return ref, obj

def handleType(type):
    ref, obj = interpretPtr(type)
    if obj:
        global types
        types[ref] = obj

def printType(outFile, typeObj):
    name = typeObj['description']
    print >>outFile, 't_%s [label="Type: %s"]' % (ref, name)

def handleNode(node):
    global totalChildLinks
    global totalRefLinks
    global totalPropLinks
    global totalExplLinks
    ref, obj = interpretPtr(node)
    if obj:
        global nodes
        nodes[ref] = obj

        for child in obj['children']:
            if handleNode(child):
                totalChildLinks += 1
        for child in obj['referredNodes']:
            if handleNode(child):
                totalRefLinks += 1
        handleType(obj['type'])
        if handleNode(obj['explanation']):
            totalExplLinks += 1

        # Also walk the properties; look for nodes
        for prop in obj['properties']:
            if prop['kind'] == 2:
                if handleNode(prop['val']):
                    totalPropLinks += 1
            if prop['kind'] == 3:
                handleType(prop['val'])
        return True
    return False

def printLinks(outFile, srcRef, destRefs, style):
    if len(destRefs) > 0:
        print >>outFile, 'n_%s -> {' % srcRef,
        for r in destRefs:
            print >>outFile, 'n_%s' % r,
        print >>outFile, '}', style

def printNode(outFile, ref, level, expand):
    global nodesToExpand
    global nodes
    global expandedChildLinks
    global expandedRefLinks
    global expandedPropLinks
    global expandedExplLinks

    if ref in nodesExpanded:
        return
    nodesExpanded[ref] = 1

    nodeObj = nodes[ref]
    name = getNodeName(nodeObj['kindName'], nodeObj)
    if args.showNodeIds:
        print >>outFile, 'n_%s [label=<<FONT POINT-SIZE="10">%s</FONT><BR/>%s >]' % (ref, ref, cgi.escape(name).encode('ascii', 'xmlcharrefreplace'))
    else:
        print >>outFile, 'n_%s [label="%s"]' % (ref, name)

    if not expand or level > args.maxLevels:
        return

    # Print links to the children nodes
    childRefs = []
    for child in nodeObj['children']:
        childRef, childObj = interpretPtr(child)
        if childRef and expandedChildLinks < args.maxChildNodes:
            nodesToExpand.append((childRef, level+1, True))
            childRefs.append(childRef)
            expandedChildLinks += 1
    printLinks(outFile, ref, childRefs, '')

    # Print links to the referred nodes
    childRefs = []
    for child in nodeObj['referredNodes']:
        childRef, childObj = interpretPtr(child)
        if childRef and expandedRefLinks < args.maxReferredNodes:
            nodesToExpand.append((childRef, level+1, False))
            childRefs.append(childRef)
            expandedRefLinks += 1
    printLinks(outFile, ref, childRefs, '[style="dashed", constraint=false]')

    # Print links to the nodes in the properties
    childRefs = []
    for prop in nodeObj['properties']:
        if prop['kind'] == 2:
            otherNode = prop['val']
            childRef, childObj = interpretPtr(otherNode)
            if childRef and expandedPropLinks < args.maxPropNodes:
                nodesToExpand.append((childRef, level+1, False))
                childRefs.append(childRef)
                expandedPropLinks += 1
    printLinks(outFile, ref, childRefs, '[style="dotted", constraint=false]')

    # Print connections with the explanation
    explanationRef, explanationObj = interpretPtr(nodeObj['explanation'])
    if explanationRef and expandedExplLinks < args.maxExplNodes:
        nodesToExpand.append((explanationRef, level+1, True))
        print >>outFile, 'n_%s -> n_%s [style=bold, arrowhead=vee, arrowtail=inv, dir=both]' % (ref, explanationRef)
        expandedExplLinks += 1


def printAsTree(outFile, jsonData):
    print >>outFile, """digraph tree
    {
        nodesep=0.5;   

        charset="latin1";
        rankdir=LR;
        fixedsize=true;
        node [style="rounded,filled", width=0, height=0, shape=box, fillcolor="#E5E5E5", concentrate=true]

    """
    # Handle all the nodes, without printing anything
    handleNode(jsonData)

    # The start node for expansion
    if args.startNode:
        nodesToExpand.append((args.startNode, 1, True))
    else:
        ref, obj = interpretPtr(jsonData)
        if ref:
            nodesToExpand.append((ref, 1, True))

    # Now do the printing of all the relevant nodes
    numExpanded = 0
    while len(nodesToExpand) > 0 and numExpanded < args.maxNodes:
        firstNode = nodesToExpand.pop()
        printNode(outFile, firstNode[0], firstNode[1], firstNode[2])
        numExpanded += 1

    print >>outFile, '}'


def main():
    print 'gen-tree, copyright (c) 2015 Lucian Radu Teodorescu'
    print ''

    parseArgs()

    jsonContent = getFileContents(args.jsonFile)
    jsonData = json.loads(jsonContent)
    # print jsonData

    outFilename = args.jsonFile + '.dot'
    with open(outFilename, 'w') as outFile:
        printAsTree(outFile, jsonData)

    print 'Nodes: %d' % len(nodes)
    print 'Types: %d' % len(types)
    print 'Links child: %d/%d' % (expandedChildLinks, totalChildLinks)
    print 'Links ref: %d/%d' % (expandedRefLinks, totalRefLinks)
    print 'Links prop: %d/%d' % (expandedPropLinks, totalPropLinks)
    print 'Links expl: %d/%d' % (expandedExplLinks, totalExplLinks)

    if args.open:
        # Create .pdf out of the .dot file
        pdfFilename = args.jsonFile + '.pdf'
        with open(outFilename) as inFile:
            with open(pdfFilename, 'w') as pdfFile:
                process = subprocess.Popen(['dot', '-Tpdf'], stdin=inFile, stdout=pdfFile)
                res = process.wait()
                if res != 0:
                    print 'ERROR executing dot command'
                    return res
        # Open the .pdf file
        subprocess.call(['open', pdfFilename])

if __name__ == "__main__":
    main()
