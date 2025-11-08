CHUNK = 1 * 1024 * 1024  # 1 MiB
TOTAL = 100 * CHUNK
with open("100mb.bin", "wb") as f:
    written = 0
    chunk = b"a" * CHUNK
    while written < TOTAL:
        f.write(chunk)
        written += CHUNK
