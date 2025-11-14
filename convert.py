#!/usr/bin/env python3

# u can move this file through this project by copy this file to directory u need to convert .dae(zip) to .dae
import os
import shutil
import gzip
import zipfile

SRC_DIR = "."
OUT_DIR = "./convert"

os.makedirs(OUT_DIR, exist_ok=True)


def is_gzip(path: str) -> bool:
    with open(path, "rb") as f:
        return f.read(2) == b'\x1f\x8b'


def is_zip(path: str) -> bool:
    with open(path, "rb") as f:
        return f.read(4) == b'PK\x03\x04'


def handle_zip(path: str, output_name: str):
    print(f"[ZIP] Extracting ZIP-compressed DAE: {os.path.basename(path)}")

    with zipfile.ZipFile(path, 'r') as z:
        members = z.namelist()

        if len(members) == 0:
            print("   ERROR: ZIP has no entries!")
            return

        # Assume the first file is the DAE
        internal_name = members[0]

        print(f"       Extracted: {internal_name}")

        tmp_path = z.extract(internal_name, OUT_DIR)

        # Rename extracted file to match original filename
        final_path = os.path.join(OUT_DIR, output_name)
        os.replace(tmp_path, final_path)
        print(f"       Renamed to: {output_name}")


def handle_gzip(path: str, output_name: str):
    print(f"[GZIP] Decompressing: {output_name}")
    with gzip.open(path, "rb") as gz:
        data = gz.read()
        with open(os.path.join(OUT_DIR, output_name), "wb") as out:
            out.write(data)


def handle_copy(src: str, name: str):
    print(f"[COPY] Normal DAE: {name}")
    shutil.copy2(src, os.path.join(OUT_DIR, name))


# Process all .dae files
for filename in os.listdir(SRC_DIR):
    if not filename.lower().endswith(".dae"):
        continue

    src_path = os.path.join(SRC_DIR, filename)

    if is_zip(src_path):
        handle_zip(src_path, filename)
        continue

    if is_gzip(src_path):
        handle_gzip(src_path, filename)
        continue

    handle_copy(src_path, filename)

print("Done! Files are in ./convert")
