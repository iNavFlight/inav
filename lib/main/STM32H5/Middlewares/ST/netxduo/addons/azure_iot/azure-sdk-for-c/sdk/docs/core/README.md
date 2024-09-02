# Azure SDK Core Library for Embedded C

Azure Core Library for Embedded C (`az_core`) provides shared primitives, abstractions, and helpers for modern Azure SDK client libraries written in the C programming language. These libraries follow the Azure SDK Design Guidelines for Embedded C.

The library allows client libraries to expose common functionality in a consistent fashion.  Once you learn how to use these APIs in one client library, you will know how to use them in other client libraries.

## Porting the Azure SDK to Another Platform

The `Azure Core` library requires you to implement a few functions to provide platform-specific features such as a clock and thread sleep. By default, `Azure Core` ships with no-op versions of these functions, all of which return `AZ_ERROR_DEPENDENCY_NOT_PROVIDED`. These function versions allow the Azure SDK to compile successfully so you can verify that your build tool chain is working properly; however, failures may occur if you execute the code.

## Key Concepts

### Function Results

Many SDK functions return an `az_result` as defined in [inc/az_result.h](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/inc/azure/core/az_result.h) header file. An `az_result` is a 32-bit enum value. When a function succeeds, it typically returns AZ_OK. When a function fails, it returns an `az_result` symbol prefixed with `AZ_ERROR_`. A few functions return a reason for success; these symbols will be prefixed with `AZ_` but will **not** contain `ERROR` in the symbol. For functions that need to return an `az_result` and some other value; the other value is returned via an output parameter. If you simply want to know if an `az_result` value indicates generic success or failure, call either the `az_result_succeeded` or `az_result_failed` function, respectively. Both of these functions take an `az_result` value and return `true` or `false`.

### Working with Spans

An `az_span` is a small data structure (defined in our [az_span.h](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/inc/azure/core/az_span.h) file) wrapping a byte buffer. Specifically, an `az_span` instance contains:

- a byte pointer
- an integer size

Our SDK passes `az_span` instances to functions to ensure that a buffer's address and size are always passed together; this reduces the chance of bugs. And, since we have the size, operations are fast; for example, we never need to call `strlen` to find the length of a string in order to append to it. Furthermore, when our SDK functions write or copy to an `az_span`, our functions ensure that we never write beyond the size of the buffer; this prevents data corruption. And finally, when reading from an `az_span`, we never read past the `az_span`'s size ensuring that we don't process uninitialized data.

Since many of our SDK functions require `az_span` parameters, customers must know how to create `az_span` instances so that you can call functions in our SDK. Here are some examples.

Create an empty `az_span`:

```C
az_span empty_span = AZ_SPAN_EMPTY; // size = 0
```

Create an `az_span` expression from a byte buffer:

```C
uint8_t buffer[1024];
some_function(AZ_SPAN_FROM_BUFFER(buffer));  // size = 1024
```

Create an `az_span` literal from a string (the span does NOT include the 0-terminating byte):

```C
az_span span_over_str = AZ_SPAN_LITERAL_FROM_STR("Hello");  // size = 5
```

Create an `az_span` expression from a string (the span does NOT include the 0-terminating byte):

```C
some_function(AZ_SPAN_FROM_STR("Hello"));  // size = 5
```

As shown above, an `az_span` over a string does not include the 0-terminator. If you need to 0-terminate the string, you can call this function to append a 0 byte (if the span's size is large enough to hold the extra byte):

```C
az_span az_span_copy_u8(az_span destination, uint8_t byte);
```

and then call this function to get the address of the 0-terminated string:

```C
char* str = (char*) az_span_ptr(span); // str points to a 0-terminated string
```

Or, you can call this function to copy the string in the `az_span` to your own `char*` buffer; this function will 0-terminate the string in the `char*` buffer:

```C
void az_span_to_str(char* destination, int32_t destination_max_size, az_span source);
```

There are many functions to manipulate `az_span` instances. You can slice (subset an `az_span`), parse an `az_span` containing a string into an number, format a number as a string into an `az_span`, check if two `az_span` instances are equal or the contents of two `az_span` instances are equal, and more.

### Strings

A string is a span of UTF-8 characters. It's not a zero-terminated string. Defined in [inc/az_span.h](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/inc/azure/core/az_span.h).

```c
az_span hello_world = AZ_SPAN_FROM_STR("Hello world!");
```

### Logging SDK Operations

As our SDK performs operations, it can send log messages to a customer-defined callback. Customers can enable this to assist with debugging and diagnosing issues when leveraging our SDK code.

To enable logging, you must first write a callback function that our logging mechanism will invoke periodically with messages. The function signature must match this type definition (defined in the [az_log.h](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/inc/azure/core/az_log.h) file):

   ```C
   typedef void (*az_log_message_fn)(az_log_classification classification, az_span message);
   ```

And then, during your application's initialization, you must register your function with our SDK by calling this function:

   ```C
   void az_log_set_message_callback(az_log_message_fn log_message_callback);
   ```

This will log messages for all classifications. If you are only interested in certain kinds of messages, you can implement the following callback function which will let you filter the types of messages your `az_log_message_fn` will receive.

   ```C
   typedef bool (*az_log_classification_filter_fn)(az_log_classification classification);
   ```

And then, during your application's initialization, you can register this function with our SDK by calling this function:

   ```C
   void az_log_set_classification_filter_callback(az_log_classification_filter_fn message_filter_callback);
   ```

Now, whenever our SDK wants to send a log message, it will invoke your callback function passing it the log classification and an `az_span` containing the message string (not 0-terminated). Your callback method can now do whatever it wants to with this message such as append it to a file or write it to the console.

**Note:** In a multi-threaded application, multiple threads may invoke this callback function simultaneously; if your function requires any kind of thread synchronization, then you must provide it.

Log classifications allow your application to select which specific log messages it wants to receive. Here is a complete example that logs HTTP request and response messages to standard output:

   ```C
   static void write_log_message(az_log_classification classification, az_span message)
   {
      (void)classification;
      printf("%.*s\n", az_span_size(message), az_span_ptr(message));
   }

   static bool should_write_log_message(az_log_classification classification)
   {
      switch (classification)
      {
         case AZ_LOG_HTTP_REQUEST:
         case AZ_LOG_HTTP_RESPONSE:
            return true;
         default:
            return false;
      }
   }

   int main()
   {
      az_log_set_message_callback(write_log_message);
      az_log_set_classification_filter_callback(should_write_log_message);

      // More code goes here...
   }
   ```

If the SDK is built with `AZ_NO_LOGGING` macro defined (or adding option -DLOGGING=OFF with CMake), it should reduce the binary size and slightly improve performance.
Logging has a negligible performance impact if no listener is registered or if your filter allows few classifications. However, if you'd like to exclude all of the logging code to make your final executable smaller, define the `AZ_NO_LOGGING` symbol when building the SDK.

### SDK Function Argument Validation

The public SDK functions validate the arguments passed to them to ensure that the calling code is passing valid values. The valid value is called a contract precondition. If an SDK function detects a precondition failure (invalid argument value), then by default, it calls a function that places the calling thread into an infinite sleep state; other threads continue to run.

To override the default behavior, implement a function matching the `az_precondition_failed_fn` function signature and then, in your application's initialization (before calling any Azure SDK function), call `az_precondition_failed_set_callback` passing it the address of your function. Now, when any Azure SDK function detects a precondition failure, it will invoke your callback instead. You might override the callback to attach a debugger or perhaps to reboot the device rather than allowing it to continue running with unpredictable behavior.

Also, if you define the `AZ_NO_PRECONDITION_CHECKING` symbol when compiling the SDK code (or adding option -DPRECONDITIONS=OFF with cmake), all of the Azure SDK precondition checking will be excluded, making the binary code smaller and faster. We recommend doing this before you ship your code.

### Canceling an Operation

`Azure Core` provides a rich cancellation mechanism by way of its `az_context` type (defined in the [az_context.h](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/inc/azure/core/az_context.h) file). As your code executes and functions call other functions, a pointer to an `az_context` is passed as an argument through the functions. At any point, a function can create a new `az_context` specifying a parent `az_context` and a timeout period and then, this new `az_context` is passed down to more functions. When a parent `az_context` instance expires or is canceled, all of its children are canceled as well.

There is a special singleton instance of the `az_context` type called `az_context_application`. This instance represents your entire application and this `az_context` instance never expires. It is common to use this instance as the ultimate root of all `az_context` instances. So then, as functions call other functions, these functions can create child `az_context` instances and pass the child down through the call tree. Imagine you have the following `az_context` tree:

- `az_context_application`; never expires
  - `az_context_child`; expires in 10 seconds
    - `az_context_grandchild`; expires in 60 seconds

Any code using `az_context_grandchild` expires in 10 seconds (not 60 seconds) because it has a parent that expires in 10 seconds. In other words, each child can specify its own expiration time but when a parent expires, all its children also expire. While `az_context_application` never expires, your code can explicitly cancel it thereby canceling all the children `az_context` instances. This is a great way to cleanly cancel all operations in your application allowing it to terminate quickly.

Note however that cancellation is performed as a best effort; it is not guaranteed to work in a timely fashion. For example, the HTTP stack that you use may not support cancellation. In this case, cancellation will be detected only after the I/O operation completes or before the next I/O operation starts.

   ```C
   // Some function creates a child with a 10-second expiration:
   az_context child = az_context_create_with_expiration(&az_context_application, 10);

   // Some function creates a grandchild with a 60-second expiration:
   az_context grandchild = az_context_create_with_expiration(&child, 60);

   // Some other function (perhaps in response to a SIGINT) cancels the application root:
   az_context_cancel(&az_context_application);
   // All children are now in the canceled state & the threads will start unwinding
   ```

## Contributing

If you'd like to contribute to this library, please read the [contributing guide][azure_sdk_for_c_contributing] to learn more about how to build and test the code.

### License

Azure SDK for Embedded C is licensed under the [MIT][azure_sdk_for_c_license] license.

<!-- LINKS -->
[azure_sdk_for_c_contributing]: https://github.com/Azure/azure-sdk-for-c/blob/main/CONTRIBUTING.md
[azure_sdk_for_c_license]: https://github.com/Azure/azure-sdk-for-c/blob/main/LICENSE
[azure_sdk_for_c_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-c/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_c_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-c/blob/main/CONTRIBUTING.md#pull-requests
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_pattern_circuit_breaker]: https://docs.microsoft.com/azure/architecture/patterns/circuit-breaker
[azure_pattern_retry]: https://docs.microsoft.com/azure/architecture/patterns/retry
[azure_portal]: https://portal.azure.com
[azure_sub]: https://azure.microsoft.com/free/
[c_compiler]: https://visualstudio.microsoft.com/vs/features/cplusplus/
[cloud_shell]: https://docs.microsoft.com/azure/cloud-shell/overview
[cloud_shell_bash]: https://shell.azure.com/bash
