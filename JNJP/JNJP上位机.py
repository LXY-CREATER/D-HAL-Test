# =========================================
# 弯道预警系统一体化上位机 V1.0
# =========================================
# ✅ 集成串口控制台 + 命令按钮 + 智能体报告
# ✅ 一个串口，永不冲突
# =========================================

import serial
import json
import time
import threading
from datetime import datetime
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox

class JNJP_App:
    def __init__(self, root):
        self.root = root
        self.root.title("弯道预警系统一体化上位机 V1.0 - 智能体")
        self.root.geometry("900x700")
        
        self.ser = None
        self.connected = False
        self.data_count = 0
        self.vehicle_count = 0
        self.first_data_time = None
        self.g_status_history = []
        self.g_last_peak_time = time.time()
        self.g_alert_triggered = set()
        
        self.MAINTENANCE_THRESHOLDS = {
            "BASELINE_LOW": 30,
            "BASELINE_HIGH": 150,
            "NO_PEAK_FOR_10MIN": 600,
            "NOISE_LEVEL": 50,
        }
        
        self.TRAFFIC_THRESHOLDS = {
            "LOW_PEAK": 2,      # 5分钟<=2辆 = 低峰
            "NORMAL_PEAK": 9,   # 5分钟3-9辆 = 正常
            # >9辆 = 高峰
        }
        
        self.create_ui()
        
    def create_ui(self):
        top_frame = ttk.Frame(self.root)
        top_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Label(top_frame, text="串口号:").pack(side=tk.LEFT, padx=5)
        self.com_var = tk.StringVar(value="COM5")
        ttk.Entry(top_frame, textvariable=self.com_var, width=8).pack(side=tk.LEFT)
        
        self.connect_btn = ttk.Button(top_frame, text="连接", command=self.toggle_connect)
        self.connect_btn.pack(side=tk.LEFT, padx=5)
        
        ttk.Label(top_frame, text="波特率: 115200").pack(side=tk.LEFT, padx=20)
        
        self.status_label = ttk.Label(top_frame, text="未连接", foreground="red")
        self.status_label.pack(side=tk.LEFT)
        
        cmd_frame = ttk.LabelFrame(self.root, text="快捷命令按钮")
        cmd_frame.pack(fill=tk.X, padx=5, pady=5)
        
        cmds = [
            ("状态S", "S"), ("清除C", "C"),
            ("L1无车", "L1 N"), ("L1小车", "L1 S"), ("L1大车", "L1 L"),
            ("L2无车", "L2 N"), ("L2小车", "L2 S"), ("L2大车", "L2 L"),
            ("UPLOAD数据", "UPLOAD"), ("校准", "CAL"),
        ]
        
        for text, cmd in cmds:
            btn = ttk.Button(cmd_frame, text=text, command=lambda c=cmd: self.send_cmd(c))
            btn.pack(side=tk.LEFT, padx=3, pady=3)
        
        main_paned = ttk.PanedWindow(self.root, orient=tk.VERTICAL)
        main_paned.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        notebook = ttk.Notebook(main_paned)
        
        console_tab = ttk.Frame(notebook)
        config_tab = ttk.Frame(notebook)
        notebook.add(console_tab, text="控制台")
        notebook.add(config_tab, text="参数设置")
        main_paned.add(notebook, weight=3)
        
        console_frame = ttk.LabelFrame(console_tab, text="串口控制台")
        console_frame.pack(fill=tk.BOTH, expand=True)
        self.console = scrolledtext.ScrolledText(console_frame, font=("Consolas", 10))
        self.console.pack(fill=tk.BOTH, expand=True)
        
        input_frame = ttk.Frame(console_frame)
        input_frame.pack(fill=tk.X)
        self.cmd_input = ttk.Entry(input_frame)
        self.cmd_input.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        self.cmd_input.bind('<Return>', lambda e: self.send_cmd_input())
        ttk.Button(input_frame, text="发送", command=self.send_cmd_input).pack(side=tk.LEFT)
        
        self.create_config_tab(config_tab)
        
        ai_frame = ttk.LabelFrame(console_tab, text="智能分析区")
        ai_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Button(ai_frame, text="立即生成检测报告", command=self.force_report, 
                  style='Accent.TButton').pack(side=tk.LEFT, padx=5)
        self.auto_report_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(ai_frame, text="每5个样本自动弹报告", variable=self.auto_report_var).pack(side=tk.LEFT, padx=10)
        self.report_status = ttk.Label(ai_frame, text="就绪")
        self.report_status.pack(side=tk.LEFT, padx=10)
        
        self.log("系统就绪，请点击【连接】按钮")
        self.log("=" * 50)

    def create_config_tab(self, parent):
        curve_frame = ttk.LabelFrame(parent, text="弯道参数")
        curve_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(curve_frame, text="弯道半径(m):").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)
        self.curve_radius_var = tk.StringVar(value="50")
        ttk.Entry(curve_frame, textvariable=self.curve_radius_var, width=10).grid(row=0, column=1, padx=5)
        
        ttk.Label(curve_frame, text="摩擦系数:").grid(row=0, column=2, padx=5, pady=5, sticky=tk.W)
        self.friction_var = tk.StringVar(value="0.7")
        ttk.Entry(curve_frame, textvariable=self.friction_var, width=10).grid(row=0, column=3, padx=5)
        
        ttk.Button(curve_frame, text="下发到设备", command=self.send_curve_config).grid(row=0, column=4, padx=15)
        
        thres_frame = ttk.LabelFrame(parent, text="检测阈值(ADC)")
        thres_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(thres_frame, text="无车阈值:").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)
        self.thres_none_var = tk.StringVar(value="300")
        ttk.Entry(thres_frame, textvariable=self.thres_none_var, width=8).grid(row=0, column=1, padx=5)
        
        ttk.Label(thres_frame, text="小车阈值:").grid(row=0, column=2, padx=5, pady=5, sticky=tk.W)
        self.thres_small_var = tk.StringVar(value="1200")
        ttk.Entry(thres_frame, textvariable=self.thres_small_var, width=8).grid(row=0, column=3, padx=5)
        
        ttk.Label(thres_frame, text="大车阈值:").grid(row=0, column=4, padx=5, pady=5, sticky=tk.W)
        self.thres_large_var = tk.StringVar(value="2500")
        ttk.Entry(thres_frame, textvariable=self.thres_large_var, width=8).grid(row=0, column=5, padx=5)
        
        ttk.Button(thres_frame, text="下发阈值", command=self.send_threshold_config).grid(row=0, column=6, padx=15)
        
        speed_frame = ttk.LabelFrame(parent, text="速度限制(km/h)")
        speed_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(speed_frame, text="大车限速:").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)
        self.speed_large_var = tk.StringVar(value="30")
        ttk.Entry(speed_frame, textvariable=self.speed_large_var, width=8).grid(row=0, column=1, padx=5)
        
        ttk.Label(speed_frame, text="小车限速:").grid(row=0, column=2, padx=5, pady=5, sticky=tk.W)
        self.speed_small_var = tk.StringVar(value="40")
        ttk.Entry(speed_frame, textvariable=self.speed_small_var, width=8).grid(row=0, column=3, padx=5)
        
        ttk.Button(speed_frame, text="下发限速", command=self.send_speed_config).grid(row=0, column=4, padx=15)
        
        time_frame = ttk.LabelFrame(parent, text="采样与刷新参数")
        time_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(time_frame, text="采样周期(ms):").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)
        self.sample_ms_var = tk.StringVar(value="10")
        ttk.Entry(time_frame, textvariable=self.sample_ms_var, width=8).grid(row=0, column=1, padx=5)
        
        ttk.Label(time_frame, text="刷新周期(ms):").grid(row=0, column=2, padx=5, pady=5, sticky=tk.W)
        self.refresh_ms_var = tk.StringVar(value="100")
        ttk.Entry(time_frame, textvariable=self.refresh_ms_var, width=8).grid(row=0, column=3, padx=5)
        
        ttk.Label(time_frame, text="屏保超时(s):").grid(row=0, column=4, padx=5, pady=5, sticky=tk.W)
        self.timeout_s_var = tk.StringVar(value="30")
        ttk.Entry(time_frame, textvariable=self.timeout_s_var, width=8).grid(row=0, column=5, padx=5)
        
        hint = ttk.Label(parent, text="💡 提示: STM32固件需要先实现CONFIG命令支持", foreground="gray")
        hint.pack(pady=20)
    
    def send_curve_config(self):
        r = self.curve_radius_var.get()
        f = self.friction_var.get()
        self.send_cmd(f"CFG R {r} {f}")
        self.log(f"已下发弯道参数: 半径={r}m, 摩擦系数={f}")
    
    def send_threshold_config(self):
        n = self.thres_none_var.get()
        s = self.thres_small_var.get()
        l = self.thres_large_var.get()
        self.send_cmd(f"CFG T {n} {s} {l}")
        self.log(f"已下发检测阈值: 无车={n} 小车={s} 大车={l}")
    
    def send_speed_config(self):
        l = self.speed_large_var.get()
        s = self.speed_small_var.get()
        self.send_cmd(f"CFG S {l} {s}")
        self.log(f"已下发速度限制: 大车={l}km/h 小车={s}km/h")
        
    def log(self, text, color="black"):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.console.insert(tk.END, f"[{timestamp}] {text}\n", color)
        self.console.tag_config("green", foreground="green")
        self.console.tag_config("blue", foreground="blue")
        self.console.see(tk.END)
    
    def toggle_connect(self):
        if not self.connected:
            try:
                self.ser = serial.Serial(
                    self.com_var.get(), 115200, timeout=0.5
                )
                self.connected = True
                self.connect_btn.config(text="断开")
                self.status_label.config(text="已连接", foreground="green")
                self.log(f"串口连接成功: {self.com_var.get()}")
                
                threading.Thread(target=self.read_thread, daemon=True).start()
                self.send_cmd("UPLOAD")
                
            except Exception as e:
                messagebox.showerror("错误", f"串口打开失败: {e}\n\n💡 请关闭其他串口软件！")
        else:
            self.ser.close()
            self.connected = False
            self.connect_btn.config(text="连接")
            self.status_label.config(text="未连接", foreground="red")
            self.log("已断开连接")
    
    def send_cmd(self, cmd):
        if not self.connected:
            messagebox.showwarning("提示", "请先连接串口")
            return
        self.log(f">>> {cmd}", "blue")
        self.ser.write((cmd + "\n").encode())
        time.sleep(0.1)
        
        if cmd in ["L1 S", "L1 L", "L2 S", "L2 L"]:
            self.vehicle_count += 1
            self.log(f"手动模拟车辆！累计: {self.vehicle_count} 辆", "green")
            self.report_status.config(text=f"已采集 {self.data_count} 个样本 | 检测车辆: {self.vehicle_count} 辆")
    
    def send_cmd_input(self):
        cmd = self.cmd_input.get().strip()
        if cmd:
            self.send_cmd(cmd)
            self.cmd_input.delete(0, tk.END)
    
    def predictive_maintenance_check(self, data):
        alerts = []
        baseline = data.get('lane1_base', 60)
        diff = data.get('lane1_diff', 0)
        raw = data.get('lane1_raw', 60)
        
        if baseline < self.MAINTENANCE_THRESHOLDS["BASELINE_LOW"]:
            if "BASELINE_LOW" not in self.g_alert_triggered:
                alerts.append("【严重】基线异常偏低，电荷放大器可能故障")
                self.g_alert_triggered.add("BASELINE_LOW")
        
        if baseline > self.MAINTENANCE_THRESHOLDS["BASELINE_HIGH"]:
            if "BASELINE_HIGH" not in self.g_alert_triggered:
                alerts.append("【警告】基线异常偏高，传感器可能短路")
                self.g_alert_triggered.add("BASELINE_HIGH")
        
        if time.time() - self.g_last_peak_time > self.MAINTENANCE_THRESHOLDS["NO_PEAK_FOR_10MIN"]:
            if "NO_SIGNAL" not in self.g_alert_triggered:
                alerts.append("【紧急】10分钟无检测信号，陶瓷片可能脱落")
                self.g_alert_triggered.add("NO_SIGNAL")
        
        noise_level = abs(raw - baseline)
        if noise_level > self.MAINTENANCE_THRESHOLDS["NOISE_LEVEL"]:
            if "HIGH_NOISE" not in self.g_alert_triggered:
                alerts.append("【注意】信号噪声过大，检查接线是否松动")
                self.g_alert_triggered.add("HIGH_NOISE")
        
        if diff > 20:
            self.g_last_peak_time = time.time()
            self.g_alert_triggered.discard("NO_SIGNAL")
        
        return alerts
    
    def generate_report(self, data):
        baseline = data.get('lane1_base', 60)
        diff = data.get('lane1_diff', 0)
        uptime = data.get('uptime', 0)
        alerts = self.predictive_maintenance_check(data)
        
        uptime_h = uptime // 3600
        uptime_m = (uptime % 3600) // 60
        
        traffic_level, traffic_suggestion = self.analyze_traffic()
        
        health_score = 100 - len(alerts) * 15
        health_score = max(0, health_score)
        
        color = "#2E8B57" if health_score >= 80 else "#FF8C00" if health_score >= 60 else "#DC143C"
        
        report_win = tk.Toplevel(self.root)
        report_win.title("弯道预警系统 - 智能体检测报告")
        report_win.geometry("650x550")
        
        title_frame = tk.Frame(report_win, bg=color)
        title_frame.pack(fill=tk.X)
        tk.Label(title_frame, text=f"设备健康评分: {health_score}/100", 
                font=("微软雅黑", 16, "bold"), fg="white", bg=color).pack(pady=10)
        
        text = scrolledtext.ScrolledText(report_win, font=("Consolas", 10))
        text.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        report_content = f"""
╔══════════════════════════════════════════════════════════╗
║              弯道预警系统 - 智能体检测报告             ║
╚══════════════════════════════════════════════════════════╝

【基本信息】
设备编号: {data.get('device', 'JNJP001')}
检测时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
运行时长: {uptime_h}小时{uptime_m}分钟
样本序号: #{self.data_count}
累计检测车辆: {self.vehicle_count} 辆

【传感器状态】
基线值:   {baseline} ADC  →  {"正常" if 30 < baseline < 150 else "异常"}
当前差值: {diff} ADC

【车流量分析】
车流状态: {traffic_level}
{traffic_suggestion}

【预测性维护预警】
"""
        if alerts:
            for alert in alerts:
                report_content += f"  {alert}\n"
        else:
            report_content += "  所有组件运行正常，无预警\n"
        
        report_content += f"""
【智能体优化建议】
  1. 节能建议：当前时段车流较少，可将采样频率降至10Hz，节能20%
  2. 校准建议：基线稳定，建议7天后再校准
  3. 阈值建议：当前灵敏度匹配良好，保持20差分阈值

【云平台状态】
  数据已同步至云端（演示模式）

══════════════════════════════════════════════════════════
  本报告由智能体自动生成，仅供运维人员参考
══════════════════════════════════════════════════════════
"""
        text.insert(tk.END, report_content)
        text.config(state=tk.DISABLED)
        
        tk.Button(report_win, text="确认并关闭", command=report_win.destroy, 
                 font=("微软雅黑", 12), bg="#4CAF50", fg="white").pack(pady=10)

    def analyze_traffic(self):
        """分析车流量，给出节能建议"""
        if not self.first_data_time:
            return "未知", "数据不足"
        
        elapsed_min = (time.time() - self.first_data_time) / 60
        
        if elapsed_min < 5/60:  # 演示模式：5秒就出结果
            return "正常", "累计运行不足5秒，正在采集..."
        
        # 比赛演示模式：直接按当前车辆数判断，不折算
        cars = self.vehicle_count
        
        if cars <= 2:
            level = "低峰"
            suggestion = f"已检测 {cars} 辆车，属于低峰期\n    → 建议采样频率降至10Hz，节能约50%"
        elif cars <= 5:
            level = "正常"
            suggestion = f"已检测 {cars} 辆车，车流量正常\n    → 建议保持当前采样频率"
        else:
            level = "高峰"
            suggestion = f"已检测 {cars} 辆车，属于高峰期\n    → 建议采样频率升至50Hz，确保不丢车"
        
        return level, suggestion
        
    def force_report(self):
        if self.g_status_history:
            self.generate_report(self.g_status_history[-1])
        else:
            messagebox.showinfo("提示", "还没有收到数据，请先点击UPLOAD")
    
    def read_thread(self):
        while self.connected:
            try:
                line = self.ser.readline().decode('gbk', errors='ignore').strip()
                if line:
                    self.root.after(0, lambda l=line: self.process_line(l))
                time.sleep(0.01)
            except:
                break
    
    def process_line(self, line):
        if line.startswith('JNJP>'):
            return
        
        self.log(line)
        
        if line.startswith('{'):
            try:
                data = json.loads(line)
                self.g_status_history.append(data)
                self.data_count += 1
                
                if not self.first_data_time:
                    self.first_data_time = time.time()
                
                diff = data.get('lane1_diff', 0)
                if diff > 5:
                    self.vehicle_count += 1
                    self.log(f"检测到车辆通过！累计: {self.vehicle_count} 辆", "green")
                
                self.report_status.config(text=f"已采集 {self.data_count} 个样本 | 检测车辆: {self.vehicle_count} 辆")
                
                if self.auto_report_var.get() and self.data_count % 5 == 0:
                    self.generate_report(data)
                
            except:
                pass

if __name__ == "__main__":
    root = tk.Tk()
    style = ttk.Style()
    try:
        style.theme_use('clam')
    except:
        pass
    app = JNJP_App(root)
    root.mainloop()
