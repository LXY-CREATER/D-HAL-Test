# import re
# from collections import Counter
# import matplotlib.pyplot as plt
# import sys
#
#
# def load_and_filter_words(file_path):
#     """读取文件并过滤单字词"""
#     try:
#         with open(file_path, 'r', encoding='utf-8') as f:
#             text = f.read().lower()
#
#         # 匹配两个及以上连续汉字，过滤单字
#         words = re.findall(r'[\u4e00-\u9fff]{2,}', text)
#         return words
#
#     except FileNotFoundError:
#         print(f"错误：文件 {file_path} 不存在！")
#         sys.exit(1)
#     except Exception as e:
#         print(f"文件读取失败：{str(e)}")
#         sys.exit(1)
#
#
# def plot_word_freq(word_counts, threshold=3):
#     """绘制高频词条形图"""
#     # 过滤低频词并按频率排序
#     filtered = {k: v for k, v in word_counts.items() if v >= threshold}
#     if not filtered:
#         print("没有词语出现次数达到3次以上！")
#         return
#
#     sorted_words = sorted(filtered.items(), key=lambda x: (-x[1], x[0]))
#     words, counts = zip(*sorted_words)
#
#     # 配置中文字体（自动适配系统）
#     plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS']
#     plt.rcParams['axes.unicode_minus'] = False
#
#     # 创建图表
#     plt.figure(figsize=(10, 6))
#     bars = plt.bar(words, counts, color='#66b3ff')
#
#     # 添加数据标签
#     for bar in bars:
#         yval = bar.get_height()
#         plt.text(bar.get_x() + bar.get_width() / 2, yval + 0.1,
#                  yval, ha='center', va='bottom')
#
#     # 美化显示
#     plt.title(f'高频词语统计（出现次数≥{threshold}次）', fontsize=14)
#     plt.xlabel('词语', fontsize=12)
#     plt.ylabel('出现次数', fontsize=12)
#     plt.xticks(rotation=45, ha='right')
#     plt.tight_layout()
#     plt.show()
#
#
# if __name__ == "__main__":
#     # 文件路径（默认当前目录的a.txt）
#     file_path = "a.txt"
#
#     # 加载并过滤词语
#     words = load_and_filter_words(file_path)
#     word_counts = Counter(words)
#
#     # 打印统计结果
#     print("词语统计结果（按频率排序）：")
#     for word, count in sorted(word_counts.items(), key=lambda x: (-x[1], x[0])):
#         print(f"{word}: {count}次")
#
#     # 绘制图表
#     plot_word_freq(word_counts, threshold=3)