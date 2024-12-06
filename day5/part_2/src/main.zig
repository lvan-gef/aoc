const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();

fn SortedMap(comptime T: type) type {
    return struct {
        const Self = @This();
        const List = std.ArrayList(T);
        map: std.AutoHashMap(T, List),
        allocator: std.mem.Allocator,

        pub fn init(allocator: std.mem.Allocator) Self {
            return .{
                .map = std.AutoHashMap(T, List).init(allocator),
                .allocator = allocator,
            };
        }

        pub fn deinit(self: *Self) void {
            var it = self.map.iterator();

            while (it.next()) |entry| {
                entry.value_ptr.deinit();
            }

            self.map.deinit();
        }

        pub fn insert(self: *Self, key: T, value: T) !void {
            if (!self.map.contains(key)) {
                var list = List.init(self.allocator);
                try list.append(value);
                try self.map.put(key, list);
                return;
            }

            var list = self.map.getPtr(key).?;

            const insert_pos = for (list.items, 0..) |item, index| {
                if (value <= item) break index;
            } else list.items.len;

            try list.insert(insert_pos, value);
        }

        pub fn get(self: Self, key: T) ?[]const T {
            if (self.map.get(key)) |list| {
                return list.items;
            }

            return null;
        }

        pub fn containsValue(self: Self, key: T, search_value: T) bool {
            if (self.get(key)) |list| {
                for (list) |value| {
                    if (value == search_value) {
                        return true;
                    }
                }
            }

            return false;
        }

        pub fn print(self: Self) !void {
            var it = self.map.iterator();

            while (it.next()) |entry| {
                const key = entry.key_ptr.*;
                const list = entry.value_ptr.items;

                try stdout.print("Key {}: ", .{key});
                for (list) |value| {
                    try stdout.print("{} ", .{value});
                }
                try stdout.print("\n", .{});
            }
        }
    };
}

fn isValidReport(map: SortedMap(usize), report: *std.ArrayList(usize)) bool {
    for (report.items, 0..) |value, index| {
        if (index + 1 >= report.items.len) {
            break;
        }

        if (!map.containsValue(value, report.items[index + 1])) {
            return false;
        }
    }

    return true;
}

fn fixBroken(map: SortedMap(usize), report: *std.ArrayList(usize), index: usize) !usize {
    var attempts: usize = 0;
    const max_attempts = report.items.len * report.items.len; // Prevent infinite loops

    while (attempts < max_attempts) : (attempts += 1) {
        var made_swap = false;

        var counter = index;
        while (counter < report.items.len - 1) : (counter += 1) {
            if (!map.containsValue(report.items[counter], report.items[counter + 1])) {
                const tmp = report.items[counter];
                report.items[counter] = report.items[counter + 1];
                report.items[counter + 1] = tmp;
                made_swap = true;

                if (isValidReport(map, &report.*)) {
                    return report.items[report.items.len / 2];
                }
            }
        }

        if (!made_swap) {
            break;
        }
    }

    return error.UnableToFix;
}

fn parseReport(map: SortedMap(usize), report: *std.ArrayList(usize)) !usize {
    if (!isValidReport(map, &report.*)) {
        return try fixBroken(map, report, 0);
    }

    return 0;
}

fn getResult(allocator: std.mem.Allocator) !usize {
    var report = false;
    var result: usize = 0;

    var map = SortedMap(usize).init(allocator);
    defer map.deinit();

    var values = std.ArrayList(usize).init(allocator);
    defer values.deinit();

    var it = std.mem.splitScalar(u8, data, '\n');
    while (it.next()) |token| {
        if (token.len == 0) {
            if (report) {
                // trailing newline
                break;
            }

            report = true;
            continue;
        }

        if (report) {
            values.clearRetainingCapacity();

            var report_it = std.mem.tokenizeScalar(u8, token, ',');
            while (report_it.next()) |value| {
                try values.append(try std.fmt.parseInt(usize, value, 10));
            }

            result += try parseReport(map, &values);
        } else {
            var rules_it = std.mem.tokenizeScalar(u8, token, '|');
            const lhs = try std.fmt.parseInt(usize, rules_it.next().?, 10);
            const rhs = try std.fmt.parseInt(usize, rules_it.next().?, 10);
            try map.insert(lhs, rhs);
        }
    }

    return result;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const result: usize = try getResult(allocator);

    try stdout.print("result: {d}\n", .{result});
}
