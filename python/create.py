import msgpack

b = msgpack.packb([1, 2, 3], use_bin_type=True)

print(msgpack.dumps(b))
# x = [
#     131,
#     166,
#     115,
#     101,
#     110,
#     115,
#     111,
#     114,
#     163,
#     103,
#     112,
#     115,
#     164,
#     116,
#     105,
#     109,
#     101,
#     206,
#     80,
#     147,
#     50,
#     248,
#     164,
#     100,
#     97,
#     116,
#     97,
#     146,
#     203,
#     64,
#     72,
#     96,
#     199,
#     58,
#     188,
#     148,
#     112,
#     203,
#     64,
#     2,
#     106,
#     146,
#     230,
#     33,
#     49,
#     169,
# ]
# print(b)

# print(msgpack.unpack(x))^