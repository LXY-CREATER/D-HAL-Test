/**
 *测试位运算符的用法
*/
public class TestOperator04{
	
	public static void main(String[]args){
		int a=7; //0111
		int b=8; //1000
		System.out.println(a&b);  //结果：0
		System.out.println(a|b);  //结果；15
		System.out.println(a^b);  //结果：15
		System.out.println(~b);  //结果：-9
		
		//移位
		//乘以2，除以2，使用移位操作，最快！！！
		int c=5<<2;   //相当于:5*2*2
		System.out.println(c);
		System.out.println(40>>3);  //相当于：40/8
		
		
	}
}