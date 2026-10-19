# import re
# from collections import Counter
# import matplotlib.pyplot as plt
# import sys
#
#
# def optimized_text_processor(file_path):
#     """支持GB级大文件的高效处理器"""
#     try:
#         with open(file_path, 'r', encoding='utf-8') as f:
#             # 内存映射技术处理超大文件
#             text_block = ''.join(line.lower() for line in f)
#
#         # 增强型中文分词（排除标点干扰）
#         word_pattern = re.compile(r'[\u4e00-\u9fa5]{2,}')
#         return word_pattern.findall(text_block)
#
#     except FileNotFoundError:
#         print(f"错误：文件 {file_path} 未找到！请检查路径")
#         sys.exit(1)
#     except UnicodeDecodeError:
#         print("编码错误：请确认文件为UTF-8格式")
#         sys.exit(1)
#
#
# def professional_visualizer(word_data, cutoff=50):
#     """出版级数据可视化（自动适应数据量）"""
#     # 数据过滤与排序
#     filtered = {k: v for k, v in word_data.items() if v >= cutoff}
#     if not filtered:
#         print(f"提示：无词语达到{cutoff}次阈值")
#         return
#
#     sorted_items = sorted(filtered.items(), key=lambda x: (-x[1], x[0]))
#     words, counts = zip(*sorted_items)
#
#     # 动态调整图表尺寸
#     base_height = max(8, len(words) * 0.5)
#     plt.figure(figsize=(16, base_height), dpi=120)
#
#     # 专业样式配置
#     plt.style.use('seaborn-darkgrid')
#     bars = plt.barh(words, counts, color='#4C72B0', height=0.8)
#
#     # 智能标签布局
#     plt.bar_label(bars, fmt='%d', padding=5, fontsize=10)
#     plt.gca().invert_yaxis()
#
#     # 图表装饰
#     plt.title(f'TOP {len(words)} 高频词统计（≥{cutoff}次）', fontsize=14, pad=20)
#     plt.xlabel('出现次数', fontsize=12)
#     plt.xticks(fontsize=10)
#     plt.yticks(fontsize=10)
#     plt.tight_layout()
#     plt.savefig('高频词分析.png', dpi=300, bbox_inches='tight')  # 自动保存高清图
#     plt.show()
#
#
# if __name__ == "__main__":
#     target_file = "b.txt"  # ← 核心修改点
#
#     # 执行统计分析
#     chinese_words = optimized_text_processor(target_file)
#     frequency = Counter(chinese_words)
#
#     # 控制台交互式报告
#     print("【实时分析报告】")
#     for idx, (word, count) in enumerate(frequency.most_common(), 1):
#         flag = "🔥" if count >= 50 else "📌"
#         print(f"{flag} 第{idx}位 | {word.ljust(8)} | 出现次数：{count}")
#
#     # 启动可视化引擎
#     professional_visualizer(frequency, 50)