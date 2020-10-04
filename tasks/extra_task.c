#include "extra_task.h"

#include <stddef.h>

#include "assert.h"

#include "game_loop.h"

#define EXTRA_TASK_MAX 8

static ExtraTask tasks[EXTRA_TASK_MAX];

void et_reset() {
    for (uint32_t i = 0; i < EXTRA_TASK_MAX; i++) {
        tasks[i].handle = ET_HANDLE_FREE;
    }
}

ExtraTask *et_alloc() {
    for (uint32_t i = 0; i < EXTRA_TASK_MAX; i++) {
        ExtraTask *task = &tasks[i];
        if (!et_live(task)) {
            task->handle.index = i;
            return task;
        }
    }

    fatal_error("Failed to allocate extra task");
}

void et_free(ExtraTask *task) {
    assert(et_live(task));

    task->handle.index = ET_UNDEFINED;
    task->handle.generation++;
}

ExtraTask *et_index(uint8_t index) {
    ExtraTask *task = &tasks[index];
    return (et_live(task) ? task : NULL);
}

void et_cancel(ExtraTaskHandle handle) {
    ExtraTask *task = et_index(handle.index);
    if (!task) {
        return;
    }

    et_free(task);
}

GameLoopAction et_run() {
    GameLoopAction action = GL_ACTION_NONE;

    for (uint32_t i = 0; i < EXTRA_TASK_MAX; i++) {
        ExtraTask *task = &tasks[i];
        if (!et_live(task)) {
            continue;
        }

        GameLoopAction task_result_action = task->main(task);
        if (task_result_action != GL_ACTION_NONE) {
            // Assume that only 1 task can return an action that impacts the game loop
            // Don't want multiple tasks stepping on each other, likely a bug
            assert(action == GL_ACTION_NONE);

            action = task_result_action;
        }
    }

    return action;
}

bool et_live(const ExtraTask *task) {
    return (task->handle.index != ET_UNDEFINED);
}

bool et_handle_live(ExtraTaskHandle handle) {
    uint8_t index = handle.index;
    if (index == ET_UNDEFINED) {
        return false;
    }

    const ExtraTask *task = &tasks[index];
    if (task->handle.index == ET_UNDEFINED) {
        return false;
    }

    return (tasks[index].handle.generation == handle.generation);
}
