// ============ Available APIs ============

// -- Profiling --
int custom_lock_acquire(struct __lock_policy_args *args);
int custom_lock_contended(struct __lock_policy_args *args);
int custom_lock_acquired(struct __lock_policy_args *args);
int custom_lock_release(struct __lock_policy_args *args);


// -- Shuffling --
bool custom_cmp_func(struct __lock_policy_args *args);
bool custom_skip_func(struct __lock_policy_args *args);
bool custom_break_func(struct __lock_policy_args *args);

