#!/usr/bin/env python
from collections import defaultdict
import re
import xml.etree.ElementTree as ET
import yaml


def etna_to_yml(xml):
    defs = xml.find('functions')
    functions = defaultdict(dict)
    for f in defs.findall('function'):
        name = f.get('name')
        ret = f.find('return')
        if ret is not None:
            ret = ret.get('type')
        if ret is None:
            ret = 'void'

        params = []
        for p in f.findall('param'):
            params.append('{} {}'.format(p.get('type'), p.get('name')))

        functions[name] = [ret] + params

    return functions


def lua_to_yml(xml):
    typemap = xml.find('typemap')
    types = {}
    for t in typemap:
        name = t.get('typename')
        types[name] = t.get('C-lang', name)

    defs = xml.find('functions').find('function-defs')
    functions = defaultdict(dict)
    for f in defs.findall('function'):
        cat = f.get('category')
        ret = f.get('return')
        ret = types.get(ret, ret)
        func = f.get('name')

        params = []
        for param in f.findall('param'):
            typ = param.get('type')
            typ = types.get(typ, typ)
            name = param.get('name')
            kind = param.get('kind')
            if kind in ('array', 'reference', 'array[size]'):
                typ = typ.rstrip()
                if not typ.endswith('*') or kind == 'reference':
                    typ += ' *'
                if not 'const' in typ and param.get('input', 'false') == 'true':
                    typ = 'const ' + typ
            p = '{} {}'.format(typ, name)
            p = p.replace('* ', '*')
            params.append(p)

        args = [ret]
        args.extend(params)
        functions[cat][func] = args
    return functions


def khronos_to_yml(xml):
    def extract(node):
        return node.findtext('ptype') or node.text, node.findtext('name')

    def clean(s):
        return re.sub('\s+', ' ', s).strip()

    defs = xml.find('commands')
    functions = defaultdict(dict)
    for f in defs.findall('command'):
        proto = f.find('proto')
        ret, name = extract(proto)
        params = []
        for param in f.findall('param'):
            params.append(clean(' '.join((param.itertext()))))

        functions[name] = [ret] + params
    return functions


def to_yml(filename):
    with open(filename, 'r') as f:
        data = f.read()

    data = re.sub(' xmlns="[^"]+"', '', data, count=1)
    xml = ET.fromstring(data)

    if xml.tag == 'root':
        functions = etna_to_yml(xml)
    elif xml.tag == 'specification':
        functions = lua_to_yml(xml)
    elif xml.tag == 'registry':
        functions = khronos_to_yml(xml)
    else:
        print 'unrecognized root tag:', xml.tag

    yml = yaml.dump(dict(functions))
    with open(filename.replace('xml', 'yml'), 'w') as o:
        o.write(yml)

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print 'Usage: {} <file.xml> [file.xml...]'.format(sys.argv[0])
        sys.exit(1)
    
    for name in sys.argv[1:]:
        to_yml(name)
