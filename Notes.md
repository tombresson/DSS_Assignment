# Dev Notes


## Notes

* JSON data appears to have been documented here (by a 3rd party): https://appac.github.io/mlb-data-api-docs/
  * Actual docs appear to live at: http://statsapi.mlb.com/docs/
* ~~Design will likely be super-loop through main~~
* Thread inter-op
  * Graphics thread starts, puts `Loading` on to the display
  * Fetch thread starts to fetch and parse all the data
  * Fetch thread gives semaphore to release the graphics thread
  * Graphics thread reads a status and either proceeds to gather the data to display or displays appropriate error message
* Search tokenized JSON for the `games` array, then parse the objects inside
* Caller of the games data parser module will get back a linked-list of data which they own

## ToDos

* [x] Add `errors.h` to allow modules to throw errors codes up to the main call
* [ ] Add errors to the malloc failure cases
* [ ] Fix headers to be uniform across files
* [ ] Create doubly linked list of "game graphics structs" as a static member
* [ ] Game Display module could have an init function that passes back a handler for keypresses


## SDL Notes

* Create RWops data from memory data: https://wiki.libsdl.org/SDL_RWFromMem
*
