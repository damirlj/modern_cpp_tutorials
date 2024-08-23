package com.example.practice_kotlin.design_patterns.builder

class A private constructor(
    val id: Int? = null,
    val name: String? = null
) {

    companion object {
        class Builder() {
            private var id: Int? = null
            private var name: String? = null

            // For aggregated updates - initialize with the current one (copy-constructor)
            constructor (a: A) : this() {
                id = a.id
                name = a.name
            }

            // Setters

            fun setId(id: Int) = apply { this.id = id }

            fun setName(name: String) = apply { this.name = name }

            /**
             * For aggregated updates:
             * - create the builder with secondary c-tor, that takes the current value of outer class instance
             * - update the aggregate value with the new delta-update (not all properties are set).
             * This will be new aggregated value
             *
             * @param a The outer class instance, as aggregated value
             */
            fun update(a: A) = apply {
                a.id?.let { id = it }
                a.name?.let { name = it }
            }

            fun build() = A(id, name)
        }

        // Helper method for parsing the dynamic updates as partial one (deltas)
        fun update(curr: A, new: A) = Builder(curr).update(new).build()
    }


    override fun toString(): String {
        return "[id=$id, name=$name]"
    }
}