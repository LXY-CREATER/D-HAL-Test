# import re
# from collections import Counter
# import sys
#
#
# def count_words(file_path):
#     try:
#         # 读取文件内容（自动检测编码）
#         with open(file_path, 'r', encoding='utf-8') as file:
#             text = file.read().lower()  # 统一转为小写
#
#         # 使用正则表达式分割单词（兼容中英文）
#         words = re.findall(r'[\w\u4e00-\u9fff]+', text)  # 匹配字母、数字、汉字
#
#         # 统计词频
#         word_counts = Counter(words)
#
#         # 按频率排序（次数相同按字母排序）
#         sorted_words = sorted(word_counts.items(), key=lambda x: (-x[1], x[0]))
#
#         return sorted_words
#
#     except FileNotFoundError:
#         print(f"错误：文件 {file_path} 不存在！")
#         sys.exit(1)
#     except Exception as e:
#         print(f"读取文件失败：{str(e)}")
#         sys.exit(1)
#
#
# if __name__ == "__main__":
#     # 文件路径（默认当前目录下的a.txt）
#     file_path = "a.txt"
#
#     # 统计词频
#     result = count_words(file_path)
#
#     # 打印结果（对齐格式化）
#     max_word_length = max(len(word) for word, _ in result) if result else 0
#     print(f"\n{'词语':<{max_word_length + 2}} 出现次数")
#     print("-" * (max_word_length + 10))
#
#     for word, count in result:
#         print(f"{word:<{max_word_length + 2}} {count:>5}")