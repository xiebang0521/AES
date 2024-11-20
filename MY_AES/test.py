
while True:
    # 获取用户输入的十六进制数组，以空格分隔每个十六进制元素，例如：0F 1A 2B等
    hex_array_str = input("请输入十六进制数组（元素之间用空格隔开）：")
    # 将输入的字符串按空格分割成单个十六进制元素的字符串列表
    hex_list = hex_array_str.split()

    # 将十六进制字符串元素转换为对应的整数（你可以根据实际需求调整后续处理逻辑）
    decimal_list = [int(hex_num, 16) for hex_num in hex_list]

    print("转换后的十进制数值列表:", decimal_list)
