# COMP SCI 537 Project 4

## Student Information

- name: Hongzheng Li, Junjie Yan
- CS Login: hongzheng, jyan
- Wisc ID: hli2225, jyan244
- Email: <hongzheng@cs.wisc.edu> <jyan@cs.wisc.edu>
- Status of Implementation: All tests passed

## Implementation Status

The implementation is complete and all core functionalities work as expected.
All provided test cases pass.
No known bugs at the time of submission.

## Files Changed in `solution/`

- `kalloc.c`:

  - Added `khugeinit()`, `khugealloc()`, and `khugefree()` functions to support a separate huge page allocator for 4MB pages.
  - Used a separate freelist `khugemem` for huge pages.

- `vm.c`:

  - Modified `allocuvm()` and `deallocuvm()` to handle 4MB pages using `khugealloc` and `khugefree`.
  - Updated `mappages()` logic with `PTE_PS` flag for handling huge pages.
  - Added panic checks such as `"hugepage - remap"`.
  - Add huge page mapping to `kmap`.
  - Updated `copyuvm()` for copying huge pages to child processes.

- `proc.c`:

  - Add a global variable `thp_enabled` to indicate if THP is enabled.
  - Add two functions `setthp()` and `checkthp()` to set and get the THP status.
  - Update `userinit` to initialize `p->hugesz` to `HUGE_VA_OFFSET`.
  - Modified `growproc()` to allocate memory from huge or base pages based on flags.

- `umalloc.c`:

  - Added support for `vmalloc()` and `vfree()`, separate from `malloc()` and `free()`.
  - Added a seperate free list to manage huge pages.
  - Added THP support in `malloc()` based on threshold (1MB) and `setthp` state.

- `user.h`:

  - Declared `vmalloc()`, `vfree()`, `setthp()`, and `checkthp()` system calls.

- `sysproc.c`:

  - Implemented `sys_setthp()` and `sys_checkthp()` system calls to control and query THP status.

- `usys.S` and `syscall.c`:

  - Added entries for new system calls: `setthp`, `checkthp`.

- `syscall.h`:

  - Added syscall numbers for the new system calls.

- `defs.h`:

  - Added prototypes for `vmalloc`, `vfree`, `setthp`, `checkthp`

## Additional Notes

- `clang-format` was used to format the code base.
