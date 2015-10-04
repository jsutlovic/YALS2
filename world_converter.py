#!/usr/bin/env python

import struct
import sys
import getopt


class WorldConverter:
    CELLS_PER_ELEM = 16
    MAGIC = 0xf0de

    USAGE = """{0}: convert between YALS world formats

Usage: {0} -b from.wor to.txt
       {0} -t from.txt to.wor

       -t: convert from text world to binary world
       -b: convert from binary world to text world
"""

    def __init__(self):
        self.w = 0
        self.h = 0
        self.gen = 0
        self.state = 0
        self.on = 'O'
        self.off = 'X'

        self._data = ''
        self._wd = []

    @classmethod
    def from_text_file(cls, filename):
        try:
            return cls.from_text_str(open(filename, 'r').read())
        except IOError as e:
            sys.stdout.write("{}\n".format(e))
            sys.exit(1)

    @classmethod
    def from_text_str(cls, text_str):
        w = cls()
        w._data = text_str.split('\n')

        meta = w._data[0].strip().split(',')
        w.w = int(meta[0])
        w.h = int(meta[1])
        w.on = meta[2]
        w.off = meta[3]

        raw_cells = ''.join([l.strip() for l in w._data[1:]])
        data_len = int(len(raw_cells) / cls.CELLS_PER_ELEM) + 1
        w._wd = [0] * data_len

        i = -1
        for j, e in enumerate(raw_cells):
            if j & 0xf == 0:
                i += 1
            k = 2 * (j & 0xf)
            w._wd[i] |= (int(e == w.on) << 1) << k

        return w

    @classmethod
    def from_bin_file(cls, filename):
        try:
            return cls.from_bin_str(open(filename, 'rb').read())
        except IOError as e:
            sys.stdout.write("{}\n".format(e))
            sys.exit(1)

    @classmethod
    def from_bin_str(cls, bin_str):
        w = cls()
        w._data = bin_str

        magic, = struct.unpack('!H', w._data[:2])
        if magic != cls.MAGIC:
            raise ValueError("Invalid file")

        w.w, w.h, w.gen, w.state = struct.unpack('!LLLH', w._data[2:16])

        offset = 16
        while offset < len(w._data):
            e = struct.unpack('!L', w._data[offset:offset+4])[0]
            w._wd.append(e)
            offset += 4

        return w

    def to_text_str(self):
        meta_txt = "{},{},{},{}".format(self.w, self.h, self.on, self.off)
        data_txt = ''

        i = 0
        cc = 0
        done = False

        for i, e in enumerate(self._wd):
            for j in range(self.CELLS_PER_ELEM):
                cell = (e >> j * 2) & 0x3
                data_txt += self.on if cell else self.off

                cc += 1

                if cc % self.w == 0:
                    data_txt += '\n'

                    if cc >= self.w * self.h:
                        done = True
                        break

            if done:
                break

        return "%s\n%s" % (meta_txt, data_txt)

    def to_text_file(self, filename):
        f = open(filename, 'w')
        f.write(self.to_text_str())
        f.flush()
        f.close()

    def to_bin_str(self):
        bin_str = struct.pack(
            '!HLLLH', self.MAGIC, self.w, self.h, self.gen, self.state)

        for e in self._wd:
            bin_str += struct.pack('!L', e)

        return bin_str

    def to_bin_file(self, filename):
        f = open(filename, 'wb')
        f.write(self.to_bin_str())
        f.flush()
        f.close()

    @classmethod
    def usage(cls):
        sys.stderr.write(cls.USAGE.format(sys.argv[0]))
        return sys.exit(1)

    @classmethod
    def main(cls):
        try:
            optlist, args = getopt.getopt(sys.argv[1:], 'b:t:')
        except getopt.GetoptError as e:
            sys.stderr.write("{}\n\n".format(e))
            return cls.usage()

        if len(optlist) != 1 or len(args) != 1:
            return cls.usage()

        opt, in_file = optlist[0]
        out_file = args[0]

        if opt == '-b':
            w = cls.from_bin_file(in_file)
            w.to_text_file(out_file)
        elif opt == '-t':
            w = cls.from_text_file(in_file)
            w.to_bin_file(out_file)


if __name__ == '__main__':
    WorldConverter.main()
