"""
Normalises (desparsifies) UIDs and MIDs in a MovieLens csv.
"""

import sys

if len(sys.argv) < 3:
    print(f"Usage: {sys.argv[0]} input.csv output.csv")
    sys.exit(1)

in_fn = sys.argv[1]
out_fn = sys.argv[2]

def mktrans(l):
    vals = sorted(set(l))
    return { v : i for i, v in enumerate(vals) }

def work(lines):
    yield lines[0]
    lines = lines[1:]
    def getcol(line, i):
        return int(line.split(",")[i])

    uids = [getcol(line, 0) for line in lines]
    mids = [getcol(line, 1) for line in lines]
    uid_trans = mktrans(uids)
    mid_trans = mktrans(mids)
    for i, line in enumerate(lines):
        uid, mid, rest = line.split(",", 2)
        uid = uid_trans[int(uid)] + 1
        mid = mid_trans[int(mid)] + 1
        yield f"{uid},{mid},{rest}"


with open(in_fn) as in_f:
    with open(out_fn, "w") as out_f:
        for line in work([line.strip() for line in in_f.readlines()]):
            out_f.write(line + "\n")
