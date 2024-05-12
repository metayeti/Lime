## Bugs

1. Relative paths don't work as expected from the point of view of the manifest file:

   Expected: relative paths are calculated based on the manifest file's location
   How it works: relative paths are calculated based on pwd (working directory).