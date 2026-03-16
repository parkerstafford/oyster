#include "memory.h"

struct block_header {
    size_t size;
    uint8_t is_free;
    struct block_header *next;
};

static struct block_header *heap_start = NULL;
static size_t heap_size = 0;

void memory_init(void *start, size_t size) {
    heap_start = (struct block_header *)start;
    heap_size = size;
    
    heap_start->size = size - sizeof(struct block_header);
    heap_start->is_free = 1;
    heap_start->next = NULL;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    size = (size + 7) & ~7;
    
    struct block_header *current = heap_start;
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            if (current->size >= size + sizeof(struct block_header) + 8) {
                struct block_header *new_block = (struct block_header *)((uint8_t *)current + sizeof(struct block_header) + size);
                new_block->size = current->size - size - sizeof(struct block_header);
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = 0;
            return (void *)((uint8_t *)current + sizeof(struct block_header));
        }
        current = current->next;
    }
    
    return NULL;
}

void kfree(void *ptr) {
    if (ptr == NULL) return;
    
    struct block_header *header = (struct block_header *)((uint8_t *)ptr - sizeof(struct block_header));
    header->is_free = 1;
    
    struct block_header *current = heap_start;
    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            current->size += sizeof(struct block_header) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void *kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    void *ptr = kmalloc(total);
    if (ptr) {
        uint8_t *p = (uint8_t *)ptr;
        for (size_t i = 0; i < total; i++) {
            p[i] = 0;
        }
    }
    return ptr;
}

void *krealloc(void *ptr, size_t new_size) {
    if (ptr == NULL) return kmalloc(new_size);
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    struct block_header *header = (struct block_header *)((uint8_t *)ptr - sizeof(struct block_header));
    if (header->size >= new_size) return ptr;
    
    void *new_ptr = kmalloc(new_size);
    if (new_ptr) {
        uint8_t *src = (uint8_t *)ptr;
        uint8_t *dst = (uint8_t *)new_ptr;
        for (size_t i = 0; i < header->size; i++) {
            dst[i] = src[i];
        }
        kfree(ptr);
    }
    return new_ptr;
}
