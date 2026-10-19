/**
 *测试类型强制转化
  *测试常见错误之：溢出
*/
public class TestTypeCast{
	
	public static void main(String[]args){
		double a=3.94152;
		int b=(int)a; //结果：3 浮点数强转为整数，直接丢失小数部分
		System.out.println(b);
		
		int c=97;
		char d=(char)c;
		System.out.println(d);
		
		//强制转型，超过了表数范围，则会转成一个完全不同的值
		byte e=(byte)300;
		System.out.println(e);
		
		// 测试常见错误之：溢出
        int money=1000000000; //10亿
		int years=20;
		//返回的total是负数，超过了int的范围
		int total= money*years;
		System.out.println("total="+total);
		//返回的total仍然是负数。默认是int，因此结果会转成int值，再转成long
		long total1= money*years;
		System.out.println("total1="+total);
		//返回的total2正确：先将一个因子变成long，整个表达式发生提升
		//long total2=money*((long)year)；
		long total2= 1L*money*years;
		System.out.println("total2="+total2);
	}
}