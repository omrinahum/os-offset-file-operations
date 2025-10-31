# File Processor - Offset-Based File Operations

## Topic
**File I/O and System Calls**

## Description
A file manipulation program that reads and writes data at specific byte positions. Processes commands from a file to perform offset-based read/write operations.

## Key Concepts
- **File I/O**: `fopen()`, `fclose()`, `fread()`, `fwrite()`
- **File Positioning**: `fseek()` to navigate file offsets
- **Dynamic File Growth**: File expands when writing beyond current size
- **Command Processing**: Parse and execute text-based commands

**How it works:**
- Reads commands from `requests.txt`
- Reads/writes data from/to `data.txt` 
- Saves read results to `read_results.txt`

## How to Run

### Compile
```bash
gcc -o file_processor file_processor.c
```

### Run
```bash
./file_processor data.txt requests.txt
```

### Test
```bash
# Run the Python tester (uses test cases from tests.json)
python tester.py
```
**Note:** The tester will create its own test data and requests, not your current `data.txt` and `requests.txt` files.

## Commands

### Format
Edit `requests.txt` to include commands:

| Command | Format | Description |
|---------|--------|-------------|
| **R** | `R <start> <end>` | Read from position `start` to `end` |
| **W** | `W <offset> <text>` | Write `text` at position `offset` |
| **Q** | `Q` | Quit |

### Example `requests.txt`
```
R 0 10
W 5 HELLO
R 0 15
Q
```

## Example Run

### Initial `data.txt`
```
1234567890
```

### Commands in `requests.txt`
```
R 0 5
W 3 ABC
R 0 10
Q
```

### Output
```
=== FILE PROCESSOR ===

[READ] Read from position 0: "12345"
[WRITE] Inserted "ABC" at position 3
[READ] Read from position 0: "123ABC4567"

=== DONE ===
Check 'read_results.txt' for output
```

### `read_results.txt` (output file)
```
12345
123ABC4567
```

### Final `data.txt`
```
123ABC4567890
```

## How It Works

1. **Read Operation (R)**:
   - Reads bytes between two positions
   - Saves result to `read_results.txt`

2. **Write Operation (W)**:
   - Inserts text at specified position
   - Shifts existing content to the right
   - File grows automatically

3. **Quit (Q)**:
   - Stops processing commands

## Files
- `file_processor.c` - Main program
- `data.txt` - File to manipulate
- `requests.txt` - Commands to execute
- `read_results.txt` - Output from read operations (generated)

## System Calls
- `fopen()` - Open file streams
- `fread()` - Read data from file
- `fwrite()` - Write data to file
- `fseek()` - Move file position pointer
- `fclose()` - Close file streams
