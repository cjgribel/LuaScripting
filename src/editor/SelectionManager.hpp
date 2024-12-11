#include <deque>
#include <algorithm>
#include <iterator>

#ifndef SelectionManager_h
#define SelectionManager_h

namespace Editor {

    template <typename T>
    class SelectionManager 
    {
    private:
        std::deque<T> selectedItems; // Stores selected items in order

    public:
        // Add an item to the selection
        void add(const T& item) 
        {
            // Remove if already exists, then add to the end
            remove(item);
            selectedItems.push_back(item);
        }

        // Remove an item from the selection
        void remove(const T& item) 
        {
            selectedItems.erase(
                std::remove(selectedItems.begin(), selectedItems.end(), item),
                selectedItems.end()
            );
        }

        // Check if an item is selected
        bool contains(const T& item) const 
        {
            return std::find(selectedItems.begin(), selectedItems.end(), item) != selectedItems.end();
        }

        // Clear all selections
        void clear() 
        {
            selectedItems.clear();
        }

        // Get the last selected item
        const T& last() const 
        {
            return selectedItems.back();
        }

        // Get all items except the last one
        std::deque<T> all_except_last() const 
        {
            if (selectedItems.empty()) return {};
            return std::deque<T>(selectedItems.begin(), std::prev(selectedItems.end()));
        }

        // Iterate through selected items
        const std::deque<T>& get_all() const 
        {
            return selectedItems;
        }

        // Get number of selected items
        size_t size() const 
        {
            return selectedItems.size();
        }

        // Check if selection is empty
        bool empty() const 
        {
            return selectedItems.empty();
        }
    };

} // namespace Editor
#endif