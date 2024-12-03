const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();

pub fn check_for_removing(allocator: std.mem.Allocator, list: []const i32) !std.ArrayList(i32) {
    var empty = std.ArrayList(i32).init(allocator);
    try empty.ensureTotalCapacity(list.len - 1);

    for (0..list.len) |i| {
        empty.clearRetainingCapacity();
        try empty.appendSlice(list[0..i]);
        try empty.appendSlice(list[i + 1 ..]);

        // First check if sequence is strictly monotonic (no equal adjacent numbers)
        var has_equal = false;
        for (1..empty.items.len) |j| {
            if (empty.items[j] == empty.items[j - 1]) {
                has_equal = true;
                break;
            }
        }
        if (has_equal) {
            continue;
        }

        // Then check if differences are valid and sequence is sorted
        var valid_differences = true;
        for (1..empty.items.len) |j| {
            const diff = @abs(empty.items[j] - empty.items[j - 1]);
            if (diff < 1 or diff > 3) {
                valid_differences = false;
                break;
            }
        }

        if (valid_differences and (is_sorted(empty.items, true) or is_sorted(empty.items, false))) {
            return empty;
        }
    }

    empty.clearRetainingCapacity();
    return empty;
}

fn is_sorted(slice: []const i32, comptime ascending: bool) bool {
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

fn get_numbers(allocator: std.mem.Allocator) !std.ArrayList(std.ArrayList(i32)) {
    var list = std.ArrayList(std.ArrayList(i32)).init(allocator);
    errdefer {
        for (list.items) |item| {
            item.deinit();
        }
        list.deinit();
    }

    var it = std.mem.tokenizeScalar(u8, data, '\n');
    while (it.next()) |token| {
        var rhs = std.ArrayList(i32).init(allocator);
        errdefer rhs.deinit();

        var values = std.mem.tokenizeScalar(u8, token, ' ');
        while (values.next()) |value| {
            const int_value = try std.fmt.parseInt(i32, value, 10);
            try rhs.append(int_value);
        }

        try list.append(rhs);
    }

    return list;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const lists = try get_numbers(allocator);
    defer lists.deinit();
    defer {
        for (lists.items) |list| {
            list.deinit();
        }
    }

    var result: u32 = 0;
    for (lists.items) |value| {
        var found: bool = true;

        const new_data = try check_for_removing(allocator, value.items);
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

    try stdout.print("result: {d}\n", .{result});
}
