#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2020 Huawei Technologies Co., Ltd

import argparse
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
from elftools.elf.sections import SymbolTableSection

def get_args():
    parser = argparse.ArgumentParser(description='Show relocations in an ELF '
                                     'file with section and symbol + offset.')

    parser.add_argument('input', type=argparse.FileType('rb'),
                        help='ELF input file')

    return parser.parse_args()

def get_section_name(elf, addr):
    for section in elf.iter_sections():
        start = section['sh_addr']
        end = start + section['sh_size']
        if (addr >= start and addr < end):
            return section.name

def get_symbol_plus_offset(symbols, addr):
    prev = None
    for s in symbols:
        if (s['st_value'] > addr):
            if not prev:
                return ''
            offs = addr - prev['st_value']
            if offs:
                return f'<{prev.name} + {offs}>'
            else:
                return f'<{prev.name}>'
        prev = s

def main():
    args = get_args()
    all_symbols = []
    relocs = []

    elf = ELFFile(args.input)

    for section in elf.iter_sections():
        if isinstance(section, SymbolTableSection):
            for s in section.iter_symbols():
                if (s['st_value'] != 0 and s.name != ''  and s.name != '$a'
                    and s.name != '$d' and s.name != '$t'):
                    all_symbols.append(s)
    all_symbols = sorted(all_symbols, key=lambda sym: sym['st_value'])

    for section in elf.iter_sections():
        if isinstance(section, RelocationSection):
            for relocation in section.iter_relocations():
                relocs.append(relocation['r_offset'])
    relocs.sort()

    for addr in relocs:
        target_section_name = get_section_name(elf, addr)
        sym_offs = get_symbol_plus_offset(all_symbols, addr)
        print(f'0x{addr:x} in {target_section_name} {sym_offs}')

if __name__ == "__main__":
    main()
