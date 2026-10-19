/**
 *测试类型自动转化
 */
 public class TestTypeAutoConvert{
	 
	 public static void main(String[]args){
		 //容量小的类型可以自动转化成容量的类型
		 int a=2345;
		 long b=a;
		 //int c=b;  //long类型不能自动转化成int
		 double d=b;
		 float f=b;
		 System.out.println(a);
		 System.out.println(b);
		 System.out.println(d);
		 System.out.println(f);
		 
		 //特例：整型常量是int类型，但是可以自动转成：byte/short/char。
		 //只要不超过对应类型的表数范围
		 byte h1=123;
		 //byte h2=1234;  //1234超过了byte的表数范围
		 
		 char h3 =97+25;
		 System.out.println(h3);
		 
	 }
 }