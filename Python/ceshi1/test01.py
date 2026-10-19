# -*- coding: utf-8 -*-
import jieba
from collections import Counter
import matplotlib.pyplot as plt

# 1. 读取文件
with open("b.txt", "r", encoding="utf-8") as f:
    text = f.read()

# 2. 分词并过滤单字词
words = [word for word in jieba.lcut(text) if len(word) >= 2]

# 3. 统计词频并排序
word_counts = Counter(words)
sorted_words = sorted(word_counts.items(), key=lambda x: x[1], reverse=True)

# 4. 筛选次数100次以上的词语
high_freq = {k: v for k, v in sorted_words if v > 100}

# 5. 绘制图表
plt.figure(figsize=(15, 8))
bars = plt.bar(high_freq.keys(), high_freq.values())
plt.xticks(rotation=45, ha="right", fontsize=8)
plt.xlabel("词语", fontsize=12)
plt.ylabel("出现次数", fontsize=12)
plt.title("高频词语统计（次数>100）", fontsize=14)

# 添加数据标签
for bar in bars:
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height,
             f'{height}', ha='center', va='bottom', fontsize=8)

plt.tight_layout()
plt.savefig("word_freq.png")  # 保存图片
plt.show()

# 6. 输出排序结果
print("高频词语（次数>100）排序：")
for word, count in sorted_words:
    if count > 100:
        print(f"{word}: {count}次")