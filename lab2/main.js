Array.prototype.iterator = function() {
	return {
		array : this,
		index : 0,
		hasNext : function() {
			return this.index < array.length
		},
		next : function() {
			return array[this.index++]
		}
	}
}

var array = Array.from({length: 10}, () => Math.floor(Math.random() * 10) )
var it = array.iterator()

console.log(array)
while(it.hasNext() ) {
	console.log(it.index, "is", it.next() )
}
