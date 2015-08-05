Build with:

gcc `xml2-config --cflags --libs` -o xpath_locator xpath_locator.c
./xpath_locator '/foo[1]/bar[1]'



* ✓Allocate storage for the array of XPathFinders
* (✓)For each xpath expression passed in:
  * ✓strcpy, and save the original
  * ✓Count how many segments
  * ✓Allocation storage for each of the XPathSegFinder
  * ✓Split it on '/'
  * (✓)For each segment:
      * ✓strcpy, and save the original
      * extract and save the element_local_name
      * extract and save the namespace_uri, if there is one
      * extract and save the position


# When done:

* For each XPathFinder:
  * For each XPathSegFinder
    * deallocate string kids
    * deallocate this object
  * deallocate the string kids
  * deallocate this object
* Deallocate the array of XPathFinders.