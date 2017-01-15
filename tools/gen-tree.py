#!/usr/bin/python

"""
Description: Tool for transforming JSON dumps from Sparrow Compiler into Graphviz dot diagrams

Copyright (c) 2016, Lucian Radu Teodorescu
"""

import os, subprocess, sys, json, argparse, cgi

def parseArgs():
    def str2bool(v):
      return v.lower() in ("yes", "true", "t", "1")

    parser = argparse.ArgumentParser(description='Generates dot trees from Sparrow json dumps')
    parser.register('type','bool',str2bool)
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
    parser.add_argument('--maxLevels', metavar='N', type=int, default=30,
                       help='max levels to expand (default: 30)')
    parser.add_argument('--showReferredNodes', metavar='B', type='bool', default=False,
                       help='whether should show referred nodes (default: False)')
    parser.add_argument('--showPropNodes', metavar='B', type='bool', default=False,
                       help='whether should show properties node links (default: False)')
    parser.add_argument('--showExplanation', metavar='B', type='bool', default=True,
                       help='whether should show explanation node links (default: True)')
    parser.add_argument('--expandClassAndFun', metavar='B', type='bool', default=True,
                       help='whether to expand class and function nodes (default: True)')
    return parser.parse_args()

class AstData:
    """ Class that represents the data read from the .json file; holds all the nodes and all the types """
    def __init__(self):
        self.nodes = {}
        self.types = {}

def readNodePtr(astData, jsonNodePtr):
    """Reads a node pointer json node"""
    ref = jsonNodePtr['ref'] if jsonNodePtr.get('ref') else None
    obj = jsonNodePtr['obj'] if jsonNodePtr.get('obj') else None

    if not ref:
        return None

    # If we already have the node object for this, return it
    if ref in astData.nodes.keys():
        return astData.nodes[ref]

    if not obj:
        return None

    # Read the node
    node = Node(ref, obj, astData)
    astData.nodes[ref] = node
    return node

class Type:
    """Class that holds the type information"""

    def __init__(self, astData, jsonType):
        self.ref = jsonType['ref'] if jsonType.get('ref') else None
        self.desc = jsonType['desc'] if jsonType.get('desc') else None

        astData.types[self.ref] = self

class Node:
    """Class that holds the info corresponding to a node"""

    def __init__(self, ref, jsonNode, astData):
        self.ref = ref
        self.kind = ''
        self.name = ''
        self.isDefinition = False
        self.children = []
        self.referredNodes = []
        self.explanation = []
        self.properties = {}
        self._parseJsonNode(jsonNode, astData)

    def _parseJsonNode(self, jsonNode, astData):
        if jsonNode.get('children'):
            for child in jsonNode['children']:
                n = readNodePtr(astData, child)
                if n:
                    self.children.append(n)
        if jsonNode.get('referredNodes'):
            for child in jsonNode['referredNodes']:
                n = readNodePtr(astData, child)
                if n:
                    self.referredNodes.append(n)
        if jsonNode.get('type'):
            self.type = Type(astData, jsonNode['type'])
        if jsonNode.get('explanation'):
            self.explanation = readNodePtr(astData, jsonNode['explanation'])

        # Also walk the properties; look for nodes
        if jsonNode.get('properties'):
            for prop in jsonNode['properties']:
                val = prop['val']
                kind = prop['kind']
                if kind == 2:
                    val = readNodePtr(astData, val)
                if kind == 3:
                    val = Type(astData, val)
                if val:
                    self.properties[prop['name']] = val

        self.kind = jsonNode['kind']
        self.name = self._getName()
        self.isDefinition = self._isDefinition()

    def _getName(self):
        idx = self.kind.rfind('.')
        res = self.kind[idx+1:]
        name = self._getPropertyString('name')
        if name == '':
            name = self._getPropertyString('spr.operation')
        if name == '' and self.kind == 'typeNode':
            name = self.type.desc if self.type else ''
        return res if name == '' else res + ': ' + name

    def _getPropertyString(self, name):
        if name in self.properties.keys():
            return self.properties[name]
        return ''

    def _isDefinition(self):
        return self.kind == 'spr.sprCompilationUnit' \
            or self.kind == 'spr.package' \
            or self.kind == 'spr.using' \
            or self.kind == 'spr.sprClass' \
            or self.kind == 'spr.sprFunction' \
            or self.kind == 'spr.sprParameter' \
            or self.kind == 'fun' \
            or self.kind == 'var'

class LinkType:
    """ The type of a link """
    children = 1
    referred = 2
    prop = 3
    explanation = 4

class Filter:
    """ Class that helps us filter what nodes we want to process during an AST traversal """

    def __init__(self, args = None, showReferredNodes = True, showPropNodes = True, showExplanation = True, expandClassAndFun = True):
        if args:
            self.maxNodes = args.maxNodes
            self.maxLevels = args.maxLevels
            self.showReferredNodes = args.showReferredNodes and showReferredNodes
            self.showPropNodes = args.showPropNodes and showPropNodes
            self.showExplanation = args.showExplanation and showExplanation
            self.expandClassAndFun = args.expandClassAndFun and expandClassAndFun
        else:
            self.maxNodes = 999999
            self.maxLevels = 999999
            self.showReferredNodes = showReferredNodes
            self.showPropNodes = showPropNodes
            self.showExplanation = showExplanation
            self.expandClassAndFun = expandClassAndFun
        self.numNodes = 0

    def onTraversalStart(self):
        """ Called when the traversal starts """
        self.numNodes = 0
    def onNodeAdded(self):
        """ Called when a node is actually traversed """
        self.numNodes += 1

    def canAddNewNode(self):
        """ Called to check if we can add a new node """
        return self.numNodes < self.maxNodes

    def canAddNode(self, level, linkType):
        """ Called to check if we can add a node of the given level and link type """
        return self.numNodes < self.maxNodes \
            and level < self.maxLevels \
            and ((linkType == LinkType.children) \
                or (linkType == LinkType.referred and self.showReferredNodes) \
                or (linkType == LinkType.prop and self.showPropNodes) \
                or (linkType == LinkType.explanation and self.showExplanation))

    def canExpandNode(self, node):
        classAndFun = ['spr.sprClass', 'spr.sprFunction', 'sprClass', 'sprFunction', 'Class', 'Function' ]
        return self.expandClassAndFun or (not node.kind in classAndFun)

class AstTraversal:
    """ Class that defines a traversal over the AST """
    def __init__(self, filter):
        self.filter = filter
        self._traversed = {}
        self._toTraverse = {}

    def traverse(self, node, fun):
        self._traversed = {}
        self._toTraverse = [ (node, 0) ]
        self.filter.onTraversalStart()
        while len(self._toTraverse) > 0 and self.filter.canAddNewNode():
            nodeLevel = self._toTraverse.pop()
            self._visitNode(nodeLevel[0], nodeLevel[1], fun)

    def _visitNode(self, node, level, fun):
        if node in self._traversed.keys():
            return

        # Apply the traverse function
        fun(node)
        self._traversed[node] = True
        self.filter.onNodeAdded()

        if not self.filter.canExpandNode(node):
            return

        for n in node.children:
            self._addNewNodeToTraverse(n, level+1, LinkType.children)
        for n in node.referredNodes:
            self._addNewNodeToTraverse(n, 0, LinkType.referred)
        for kv in node.properties.items():
            k = kv[0]
            v = kv[1]
            if isinstance(v, Node):
                self._addNewNodeToTraverse(v, 0, LinkType.prop)
        self._addNewNodeToTraverse(node.explanation, level+1, LinkType.explanation)

    def _addNewNodeToTraverse(self, node, level, linkType):
        if node and node not in self._traversed.keys() and self.filter.canAddNode(level, linkType):
            self._toTraverse.append((node, level))


class DotGeneration:
    """ Class that generates the dot file corresponding to the given AST """
    def __init__(self, out, args):
        self.out = out
        self.args = args
        self.backboneNodes = {} # Children nodes + explanations
        self.visibleNodes = {} # directly reachable + 1 level of other visible nodes

    def generateDot(self, rootNode):
        print >>self.out, """digraph tree
        {
            nodesep=0.5;
            charset="latin1";
            rankdir=LR;
            fixedsize=true;
            node [style="rounded,filled", width=0, height=0, shape=box, fillcolor="#E5E5E5", concentrate=true]

        """

        def _addToDict(dict, n):
            dict[n] = True

        # Gather the nodes that are directly reachable by a children/expl traversal
        self.backboneNodes = {}
        trav = AstTraversal(Filter(self.args, False, False))
        trav.traverse(rootNode, (lambda n: _addToDict(self.backboneNodes, n)))

        # Gather the nodes that should be visible
        # Expand max one level out of the directly reachable nodes
        self.visible = {}
        f = Filter(self.args)
        f.maxLevels = 1
        trav = AstTraversal(f)
        for n in self.backboneNodes:
            trav.traverse(n, (lambda n: _addToDict(self.visible, n)))

        # Check if we have directly reachable nodes that are not visible
        for n in self.backboneNodes.keys():
            if n not in self.visible:
                print 'Node %s (%s) is reachable, but not visible!' % (n.name, n.ref)

        # Print all the visible nodes
        for n in self.visible.keys():
            self._printNode(n)

        print >>self.out, '}'

    def _printNode(self, node):
        # Print the node
        ref = node.ref
        name = node.name
        style = ''
        if self.args.showNodeIds:
            style = 'label=<<FONT POINT-SIZE="10">%s</FONT><BR/>%s >' % (ref, self._escape(name))
        else:
            style = 'label="%s"' % self._escape(name)
        if node.isDefinition:
            style += ' fillcolor=lemonchiffon'
        print >>self.out, 'n_%s [%s]' % (ref, style)

        styleChild = ''
        styleRefMain = '[style=dashed, constraint=false, minlen=1]' # Back/forth references
        styleRefAux = '[style=dashed]' # Aux nodes are drawn based on constraints
        stylePropMain = '[style=dotted]'
        stylePropAux = '[style=dotted, constraint=false, minlen=1]'
        styleExpl = '[style=bold, arrowhead=vee, arrowtail=inv, dir=both]'

        # Print links to the children nodes
        dest = [n for n in node.children if n in self.visible]
        self._printLinks(ref, dest, styleChild)

        # Print links to the referred nodes
        if self.args.showReferredNodes:
            dest = [n for n in node.referredNodes if n in self.visible]
            mainRefs = [n for n in dest if n in self.backboneNodes]
            auxRefs = [n for n in dest if n not in self.backboneNodes]
            self._printLinks(ref, mainRefs, styleRefMain)
            self._printLinks(ref, auxRefs, styleRefAux)

        # Print links to the nodes in the properties
        if self.args.showPropNodes:
            for kv in node.properties.items():
                k = kv[0]
                v = kv[1]
                if isinstance(v, Node) and v in self.visible:
                    style = stylePropAux if v in self.backboneNodes else stylePropMain
                    self._printLink(ref, v.ref, k, style)

        # Print connections with the explanation
        if self.args.showExplanation:
            if node.explanation:
                self._printLink(ref, node.explanation.ref, '', styleExpl)

    def _escape(self, name):
        res = cgi.escape(name).encode('ascii', 'xmlcharrefreplace')
        res = res.replace('\\', '\\\\')
        return res

    def _printLink(self, srcRef, destRef, name, style):
        if name != '':
            print >>self.out, 'n_%s -> n_%s [label="%s"] %s' % (srcRef, destRef, self._escape(name), style)
        else:
            print >>self.out, 'n_%s -> n_%s %s' % (srcRef, destRef, style)

    def _printLinks(self, srcRef, destNodes, style):
        if len(destNodes) > 0:
            print >>self.out, 'n_%s -> {' % srcRef,
            for d in destNodes:
                print >>self.out, 'n_%s' % d.ref,
            print >>self.out, '}', style


def main():
    print 'gen-tree, copyright (c) 2016 Lucian Radu Teodorescu'
    print ''

    args = parseArgs()

    jsonContent = ''
    with open(args.jsonFile) as f:
        jsonContent = f.read().rstrip()
    jsonData = json.loads(jsonContent)
    # print jsonData

    # Read the nodes from the Json
    astData = AstData()
    rootNode = readNodePtr(astData, jsonData)

    # If a start node was specified, use it as root
    if args.startNode and args.startNode in astData.nodes.keys():
        rootNode = astData.nodes[args.startNode]

    # Generate the .dot file
    outFilename = args.jsonFile + '.dot'
    dot = None
    with open(outFilename, 'w') as outFile:
        dot = DotGeneration(outFile, args)
        dot.generateDot(rootNode)

    print 'Nodes: %d' % len(astData.nodes)
    print 'Types: %d' % len(astData.types)
    print 'Backbone nodes: %d' % len(dot.backboneNodes)
    print 'Visible nodes: %d' % len(dot.visible)

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
