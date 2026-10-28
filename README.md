Immunology System in C (GTK3 GUI)

Course: CSA0220 - C Programming Author: Poosith Kanna S Language: C (C99) with the GTK 3 GUI library

A desktop application written in C that combines two immunology tools in one window:

Disease Diagnosis: a rule-based scoring tool for immunological diseases.
Antigen Analysis: a sequence-based heuristic that finds hydrophilic candidate regions in an amino-acid sequence.

Disclaimer: This is an educational project. It uses simple rule-based logic. It is not medical advice, not a clinical diagnosis, and not an experimentally confirmed epitope prediction. It contains no machine learning.

Features
Tab 1: Disease Diagnosis
11 tick boxes for symptoms and antibody markers (for example joint pain, butterfly rash, ANA positive, anti-dsDNA positive).
4 optional lab value fields: Rheumatoid factor, Anti-CCP, CRP and ESR. Blank fields are ignored.
Scores three conditions from 0 to 100% with a text bar and a Low / Moderate / High label:
Rheumatoid arthritis
Systemic lupus erythematosus (SLE)
Sjogren's syndrome
Shows the highest-matching condition.
Rejects non-numeric or negative lab values with an error message.
Tab 2: Antigen Analysis
Accepts a FASTA sequence (a > header line is skipped) or a plain amino-acid sequence.
Reports residue count, hydrophobicity %, and hydrophilic content %.
Lists hydrophilic candidate regions (a 6-residue window with at least 5 hydrophilic residues, positions are 1-based).
Rejects invalid amino-acid letters with an error message.
C concepts used
Concept	Where it is used
Structures (struct)	Feature, Lab and App types
Arrays of structures	Symptom and lab tables with weights
Pointers and strings	Input parsing, strchr, strlen
Dynamic memory	g_malloc0 / g_free for the cleaned sequence and region flags
Functions	One function per task (on_diagnose, on_analyze, count_in, page builders)
Loops and conditions	Scoring, window scan, region merging
Macros (#define)	Window size, residue sets, table sizes
Callbacks	GTK button-click handlers
Input validation	Checks for invalid letters and bad numbers
How the scoring works

Diagnosis: each symptom, marker and abnormal lab has a weight for each disease. A disease score is the sum of the weights that apply, divided by the maximum possible weight, as a percentage. A lab value only counts when it is above its cutoff (for example RF above 20 IU/mL), and a lab left blank is not included in the maximum.

Antigen: residues AVILMFWC count as hydrophobic and DEKRNQSTH as hydrophilic. A 6-residue window is flagged when at least 5 of its residues are hydrophilic. Overlapping flagged windows are merged into one region.

Weights and cutoffs are simplified teaching values.

Requirements
A C compiler (GCC)
GTK 3 development files
pkg-config
Windows (MSYS2)
Install MSYS2 and open the MSYS2 MINGW64 terminal.
Install the tools:
bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk3 mingw-w64-x86_64-pkg-config
Linux
bash
sudo apt install build-essential libgtk-3-dev
Build and run

Windows (MINGW64 terminal):

bash
gcc immuno_gui.c -o immuno_gui.exe `pkg-config --cflags --libs gtk+-3.0`
./immuno_gui.exe

Linux:

bash
gcc immuno_gui.c -o immuno_gui `pkg-config --cflags --libs gtk+-3.0`
./immuno_gui

Run it from the MINGW64 terminal on Windows so the GTK DLLs are found. To start it by double-clicking, add C:\msys64\mingw64\bin to your Windows PATH.

Sample inputs

Diagnosis test: tick Morning stiffness > 1 hour and Joint pain / swelling, then enter RF 45 and Anti-CCP 60. Rheumatoid arthritis should score highest.

Antigen test:

>Example_Antigen
MKVLAAGIVLLAFSEEKDRNQEKSTDNLLWFVVGAAIMKQEDRSNTHEKKDLPGAFLIVC

Error tests: MKVLXBZ (invalid letters) and abc in a lab field (not a number).

Project structure
.
├── immuno_gui.c    # complete source code
└── README.md
Limitations
Scoring weights and cutoffs are simplified and not clinically validated.
Hydrophilicity is a simple residue-class count, not a validated epitope predictor.
Only three diseases and four lab tests are included.
Possible improvements
Load disease rules from a file instead of hardcoding them.
Add more diseases and lab tests.
Add a save-results-to-file button.
Use an established antigenicity scale for the sequence analysis.