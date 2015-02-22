#!/usr/bin/env python

import argparse
import jinja2
import re
from yaml import load

split_re = re.compile(r'^(?P<type>.*?)\s*(?P<name>\w+)$')
env = jinja2.Environment(
    trim_blocks=True,
    lstrip_blocks=True,
    loader=jinja2.FileSystemLoader('template'),
)

def args(args, add_type=True):
    return ', '.join(
        '{} {}'.format(arg['type'], arg['name']) if add_type else arg['name']
        for arg in args
    )

f = '0.2f'
printf_lookup = {
    'GLbitfield': 'd',
    'GLboolean': 'd',
    'GLbyte': 'c',
    'GLubyte': 'c',
    'GLchar': 'c',
    'GLdouble': '0.2f',
    'GLenum': 'u',
    'GLfloat': '0.2f',
    'GLint': 'd',
    'GLintptr': 'd',
    'GLintptrARB': 'd',
    'GLshort': 'd',
    'GLsizei': 'd',
    'GLsizeiptr': 'd',
    'GLsizeiptrARB': 'd',
    'GLuint': 'u',
    'GLushort': 'u',
    'GLvoid': 'p',
}

def printf(args):
    types = []
    for arg in args:
        typ = arg['type']
        if '*' in typ:
            t = 'p'
        else:
            t = printf_lookup.get(typ, 'p')

        types.append(t)

    return ', '.join('%' + t for t in types)

def unconst(s):
    split = s.split(' ')
    while 'const' in split:
        split.remove('const')
    return ' '.join(split)

env.filters['args'] = args
env.filters['printf'] = printf
env.filters['unconst'] = unconst

def split_arg(arg):
    match = split_re.match(arg)
    if match:
        return match.groupdict()
    else:
        return {'type': 'unknown', 'name': arg}

def gen(files, template, guard_name, headers,
        deep=False, cats=(), ifdef=None, ifndef=None):
    funcs = {}
    formats = []
    unique_formats = set()
    for data in files:
        if deep and not isinstance(data.values()[0], list):
            functions = []
            for cat, f in data.items():
                if not cats or cat in cats:
                    functions.extend(f.items())
        else:
            functions = data.items()

        for name, args in sorted(functions):
            props = {}
            if args:
                ret = args.pop(0)
            else:
                ret = 'void'

            args = [split_arg(arg) for arg in args if not arg == 'void']
            if any(arg.get('type') == 'unknown' for arg in args):
                continue

            if args:
                args[0]['first'] = True
                args[-1]['last'] = True

            for i, arg in enumerate(args):
                arg['index'] = i

            types = '_'.join(
                arg['type'].replace(' ', '_').replace('*', '__GENPT__')
                for arg in [{'type': ret}] + args)

            props.update({
                'return': ret,
                'name': name,
                'args': args,
                'types': types,
                'void': ret == 'void',
            })
            if not types in unique_formats:
                unique_formats.add(types)
                formats.append(props)

            funcs[name] = props

    context = {
        'functions': [i[1] for i in sorted(funcs.items())],
        'formats': formats,
        'headers': headers,
        'name': guard_name,
        'ifdef': ifdef,
        'ifndef': ifndef,
    }

    t = env.get_template(template)
    return t.render(**context).rstrip('\n')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate code with yml/jinja.')
    parser.add_argument('yaml', help='spec files')
    parser.add_argument('template', help='jinja template to load')
    parser.add_argument('name', help='header guard name')
    parser.add_argument('headers', nargs='*', help='headers to include')
    parser.add_argument('--deep', help='nested definitions', action='store_true')
    parser.add_argument('--cats', help='deep category filter')
    parser.add_argument('--ifdef', help='wrap with ifdef')
    parser.add_argument('--ifndef', help='wrap with ifndef')

    args = parser.parse_args()

    files = []
    for name in args.yaml.split(','):
        with open(name) as f:
            data = load(f)
            if data:
                files.append(data)

    if args.cats:
        cats = args.cats.split(',')
    else:
        cats = None
    print gen(files, args.template, args.name,
              args.headers, args.deep, cats,
              args.ifdef, args.ifndef)
