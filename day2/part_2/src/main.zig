const std = @import("std");

pub fn getSortedListByRemovingOne(allocator: std.mem.Allocator, list: []const i32) !std.ArrayList(i32) {
    var empty = std.ArrayList(i32).init(allocator);
    try empty.ensureTotalCapacity(list.len - 1);

    for (0..list.len) |i| {
        empty.clearRetainingCapacity();
        try empty.appendSlice(list[0..i]);
        try empty.appendSlice(list[i + 1..]);

        // First check if sequence is strictly monotonic (no equal adjacent numbers)
        var has_equal = false;
        for (1..empty.items.len) |j| {
            if (empty.items[j] == empty.items[j-1]) {
                has_equal = true;
                break;
            }
        }
        if (has_equal) continue;

        // Then check if differences are valid and sequence is sorted
        var valid_differences = true;
        for (1..empty.items.len) |j| {
            const diff = @abs(empty.items[j] - empty.items[j-1]);
            if (diff < 1 or diff > 3) {
                valid_differences = false;
                break;
            }
        }

        if (valid_differences and (isSorted(empty.items, true) or isSorted(empty.items, false))) {
            return empty;
        }
    }

    empty.clearRetainingCapacity();
    return empty;
}

fn isSorted(slice: []const i32, comptime ascending: bool) bool {
    for (1..slice.len) |i| {
        if (ascending and slice[i] < slice[i - 1]) {
            return false;
        }

        if (!ascending and slice[i] > slice[i - 1]) {
            return false;
        }
    }

    return true;
}

fn readNumberPairs(allocator: std.mem.Allocator, filename: []const u8) !std.ArrayList(std.ArrayList(i32)) {
    // Create the lists
    var lhs = std.ArrayList(std.ArrayList(i32)).init(allocator);
    errdefer {
        for (lhs.items) |item| {
            item.deinit();
        }
        lhs.deinit();
    }

    // Open the file
    const file = try std.fs.cwd().openFile(filename, .{});
    defer file.close();

    // Create a buffered reader
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    var line = std.ArrayList(u8).init(allocator);
    defer line.deinit();

    while (true) {
        in_stream.readUntilDelimiterArrayList(&line, '\n', 1024) catch |err| switch (err) {
            error.EndOfStream => break,
            else => return err,
        };

        var iter = std.mem.splitScalar(u8, line.items, ' ');
        var rhs = std.ArrayList(i32).init(allocator);
        errdefer rhs.deinit();

        while (iter.next()) |part| {
            if (part.len > 0) {
                const num = try std.fmt.parseInt(i32, part, 10);
                try rhs.append(num);
            }
        }
        try lhs.append(rhs);

        line.clearRetainingCapacity();
    }

    return lhs;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const stdout_file = std.io.getStdOut().writer();
    var bw = std.io.bufferedWriter(stdout_file);
    const stdout = bw.writer();

    if (std.os.argv.len != 2) {
        try stdout.print("Expect 1 argument got: {d}\n", .{std.os.argv.len - 1});

        try bw.flush();
        std.process.exit(1);
    }

    var args = std.process.args();
    _ = args.skip();

    const lists = try readNumberPairs(allocator, args.next().?);

    var result: u32 = 0;
    for (lists.items, 0..) |v, i| {
        _ = i;
        var found: bool = true;

        const new_data = try getSortedListByRemovingOne(allocator, v.items);
        defer new_data.deinit();

        if (new_data.items.len < 1) {
            continue;
        }

        for (1..new_data.items.len) |index| {
            const tmp: u32 = @abs(new_data.items[index - 1] - new_data.items[index]);
            if (tmp < 1 or tmp > 3) {
                found = false;
                break;
            }
        }

        if (found) {
            result += 1;
        }
    }

    // free it
    for (lists.items) |list| {
        list.deinit();
    }
    lists.deinit();

    try stdout.print("result: {d}\n", .{result});
    try bw.flush();
}
