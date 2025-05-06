# Embedded System Synthesis Tool

This program implements a **constructive algorithm** for the synthesis of an embedded system based on a **conditional task graph**. It uses a **normalization-based approach** for resource allocation. The system must meet a **hard time constraint**, meaning the total execution time cannot exceed the specified maximum.

---

## How to Run

1. Open **Visual Studio**.
2. Go to **File → Open → Project/Solution**.
3. Navigate to the `projekt/projekt` folder and select the `projekt.sln` file.
4. Once the solution loads, click **Debug → Start Without Debugging** or press `Ctrl + F5`.

---

## Input File

* By default, the program uses a file named `graf.txt`.
* To change the input file:

  1. Open the project as described above.

  2. Go to **line 476** in the source code.

  3. Change the line:

     ```cpp
     graph.readFromFile("graph.20.dat");
     ```

     to use your desired file name.

  4. Rebuild the solution.

---

## Program Output

The program outputs:

* The resources assigned to each task.
* Execution times for each task.
* Total execution time.
* Total system cost.

---

## Conditional Task Graph Format

The program can read conditional task graphs that include the following types of constraints:

* `min*value*`
  → Minimum amount of data required on the edge leading to the node.

* `ex(>*value*)`
  → The number of completed tasks before proceeding to this task must be **greater than** `value`.

* `ex(<*value*)`
  → The number of completed tasks must be **less than** `value`.

* `ex(value1-value2-...-valuen)`
  → Specific tasks (`value1`, `value2`, ..., `valuen`) **must be completed** before this task.

---

