Resource list
-------------

This view will show a list of all the resources in table form for all
allocations.

The top of the view shows the carousel, described earlier.

The preferred heap and resource usage filter combo box can be used to show or
hide resources depending on their resource type or preferred heap type. By
switching all preferred heaps off, some allocations will be left. Orphaned
resources are ones where the parent allocation has been deallocated without
freeing the resource first. Other resources with a '-' don't have a parent
allocation.

The table items can be sorted by selecting one of the column headers. For
example, if the **Preferred heap** column is selected, the whole table will
be sorted by preferred heap.

The search box allows for resources to be filtered by any text which is
present in the table. Any resources which do not match the text filter will not
be displayed.

.. image:: media/snapshot/resource_list_1.png
